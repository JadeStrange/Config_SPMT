#include <IPBUS_GCU_config.h>
#include "config.h"
#include "readout.h"

using namespace gcu;
uhal::ConnectionManager IPBUS_GCU_config::manager("file://cfg/connections.xml");
IPBUS_GCU_config::IPBUS_GCU_config(const std::map<std::string, std::string> &cfg_val, std::string logDirname, int mode)
{
    m_gcuId = std::stoi(cfg_val.at("GCU_ID"));
    m_gcuIp = cfg_val.at("IP");
    m_mode = mode;
    for(int i=0; i<CAT_NUM; i++)
    {
        std::string threKey = "Thre"+std::to_string(i);
        m_threshold[i] = std::stoi(cfg_val.at(threKey));
        threKey = "Thres_HG"+std::to_string(i);
        m_thresholdHG[i] = std::stoi(cfg_val.at(threKey));
        threKey = "GCU_Auto" + std::to_string(i);
        if(cfg_val.at(threKey) == "Y" || cfg_val.at(threKey) == "-")
        {
            m_gcuAuto.push_back("0x0000");
        }
        else if(cfg_val.at(threKey) == "N")
        {
            m_gcuAuto.push_back("0xFFFF");
        }
        else
        {
            m_gcuAuto.push_back(cfg_val.at(threKey));
        }
        
        threKey = "CAT_Config"+std::to_string(i);
        m_catConfig.push_back(cfg_val.at(threKey));
    }
    m_trgMode = cfg_val.at("TrigMode_PM");
    m_disTrigger = cfg_val.at("DisTrigger");
    m_freqValue = std::stoi(cfg_val.at("FreqValue"));
    m_BEC_port = std::stoi(cfg_val.at("BEC_Port"));
    
    // m_gcuAuto = "Y";

    std::string logName = logDirname+std::to_string(m_gcuId)+".txt";
    std::cout<<"logName:"<<logName<<std::endl;
    m_logFile.open(logName, std::ios::app);
    if(!m_logFile.is_open())
    {
        std::cout<<"Unable to open logFile"<<std::endl;
    }

    char* redisIp_env = std::getenv("REDIS_IP");
    std::string redisIp;
    if (redisIp_env == nullptr) {
        std::cerr << "Environment variable REDIS_IP not set." << std::endl;
        // return 1;
        redisIp="127.0.0.1";
    }
    else{redisIp = redisIp_env;}
    char* redisPort_env = std::getenv("REDIS_PORT");
    std::string redisPort;
    if (redisPort_env == nullptr) {
        std::cerr << "Environment variable REDIS_PORT not set." << std::endl;
        // return 1;
        redisPort="8088";
    }
    else{redisPort = redisPort_env;}
    std::cout<<"REDIS_IP:"<<redisIp<<" REDIS_PORT:"<<redisPort<<std::endl;
    m_redis = new Redis(redisIp, std::stoi(redisPort));
    done_flag =false;
    config_succ = true;
    
}

IPBUS_GCU_config::~IPBUS_GCU_config()
{
   
}
void IPBUS_GCU_config::start(std::string cmd)
{
    if (cmd == "test")
        m_thread = std::make_shared<std::thread>([this]()
                                                   {connect_gcu();
                                                    stop_trigger();
                                                    elec_config(); 
                                                    start_trigger();
                                                    });
    if (cmd == "config")
        m_thread = std::make_shared<std::thread>([this](){
                                                    connect_gcu();
                                                    // reset();
                                                    stop_trigger();
                                                    elec_config();
                                                });
    if (cmd == "init")
        m_thread = std::make_shared<std::thread>([this]()
                                                   {
                                                    connect_gcu();
                                                    // wait();
                                                    synchronize();
                                                    });

    if (cmd == "start")
        m_thread = std::make_shared<std::thread>([this]()
                                                   {
                                                    connect_gcu();
                                                    // stop_trigger();
                                                    // elec_config(); 
                                                    start_trigger();
                                                    });
    if (cmd == "stop")
        m_thread = std::make_shared<std::thread>([this]()
                                                   {
                                                    connect_gcu(); 
                                                    stop_trigger();
                                                    });
                                                
}

void IPBUS_GCU_config::stop(std::string cmd)
{
    m_thread->join();
    if (cmd == "default")
    {
        stop_trigger();
        reset();
    }
    close_gcu();
}
void IPBUS_GCU_config::connect_gcu()
{
    uhal::setLogLevelTo(uhal::Error());
    std::string devname="spmt.controlhub."+std::to_string(m_gcuId); //ipbus
    // ConnectionManager manager("file://cfg/connections.xml");
    hw = new HwInterface(manager.getDevice(devname));
    std::string device_id = hw->id();
    std::cout<<"GCU_"<<m_gcuId<<" Connection Success"<<std::endl;
}
int IPBUS_GCU_config::get_flag()
{
    if(done_flag)
        return config_succ;
    else
        return -1;
}
void IPBUS_GCU_config::elec_config()  //配置时run号不更新，日志文件追加写在run号+1
{
    config_succ = true;
    done_flag = false;
    {
        // 获取当前时间戳
        auto now = std::chrono::system_clock::now();
        
        // 将时间戳转换为微秒级别的时间戳
        auto now_us = std::chrono::time_point_cast<std::chrono::microseconds>(now);
        
        // 获取时间戳的微秒数部分
        auto timestamp = now_us.time_since_epoch().count();
        // std::time_t now = std::time(nullptr);
        // // 将时间戳转换为本地时间
        // std::tm* local_time = std::localtime(&now);
        
        // // 格式化本地时间为字符串
        // char timestamp[100];
        // std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local_time);
        
        {
            m_logmutex.lock();
            m_logFile<<m_gcuId<<"|ConfigGCU_Start|"<<timestamp<<std::endl;
            m_logmutex.unlock();
        }
    }
    if(m_mode == 0)
    {
        // /*
        if(!gcu_stop_acq(hw))
        {
            std::time_t now = std::time(nullptr);
        
            // 将时间戳转换为本地时间
            std::tm* local_time = std::localtime(&now);
            
            // 格式化本地时间为字符串
            char time_str[100];
            std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
            {
                m_logmutex.lock();
                m_logFile<<m_gcuId<<"|GCU_STOP_ACQ_ERROR|"<<time_str<<std::endl;
                m_redis->set("TCP_Alert", "1");
                m_logmutex.unlock();
            }
            config_succ = false;
        }
        if(!abc_stop_acq(hw))
        {
            std::time_t now = std::time(nullptr);
        
            // 将时间戳转换为本地时间
            std::tm* local_time = std::localtime(&now);
            
            // 格式化本地时间为字符串
            char time_str[100];
            std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
            {
                m_logmutex.lock();
                m_logFile<<m_gcuId<<"|ABC_STOP_ACQ_ERROR|"<<time_str<<std::endl;
                m_redis->set("TCP_Alert", "1");
                m_logmutex.unlock();
            }
            config_succ = false;
        }
        
        
        int times = 10;
        while(times > 0)
        {
            try
            {
                hw->getNode("SPI_Master_Controller").getNode("ss").write(0xFF);
                hw->dispatch();
                hw->getNode("SPI_Master_Controller").getNode("ctrl").write(0x2820);
                hw->dispatch();
                hw->getNode("SPI_Master_Controller").getNode("ctrl").read();
                hw->dispatch();
                break;
            }
            catch(const std::exception& e)
            {
                std::cout << e.what() << std::endl;
                times--;
                if(times == 0)
                {
                    std::time_t now = std::time(nullptr);
        
                    // 将时间戳转换为本地时间
                    std::tm* local_time = std::localtime(&now);
                    
                    // 格式化本地时间为字符串
                    char time_str[100];
                    std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
                    {
                        m_logmutex.lock();
                        m_logFile<<m_gcuId<<"|SPI_MASTER_CTRL_ERROR|"<<time_str<<std::endl;
                        m_redis->set("TCP_Alert", "1");
                        m_logmutex.unlock();
                    }
                    config_succ = false;
                    break;
                }
                
            }
        }
        
        for(int i=0; i<1; i++)
        {
            int temp_cfgSuc = setThreshold(hw, i, m_disTrigger, m_gcuAuto, m_trgMode, m_thresholdHG, m_threshold, m_catConfig, m_gcuId, m_logFile, m_redis);
            if(temp_cfgSuc == false)
                config_succ = false;
        }
        // config_succ = true;
        // */
    }
    if (config_succ)
        std::cout << "GCU_" << m_gcuId << " Config Success" << std::endl;
    else
        std::cout << "GCU_" << m_gcuId << " Config Fail" << std::endl;
    if (config_succ)
    {
        std::time_t now = std::time(nullptr);
        // 将时间戳转换为本地时间
        std::tm* local_time = std::localtime(&now);
        
        // 格式化本地时间为字符串
        char time_str[100];
        std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
        {
            m_logmutex.lock();
            m_logFile<<m_gcuId<<"|Config_Success|"<<time_str<<std::endl;
            m_logmutex.unlock();
        }
    }
    else
    {
        std::time_t now = std::time(nullptr);
        // 将时间戳转换为本地时间
        std::tm* local_time = std::localtime(&now);
        
        // 格式化本地时间为字符串
        char time_str[100];
        std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
        {
            m_logmutex.lock();
            m_logFile<<m_gcuId<<"|Config_Error|"<<time_str<<std::endl;
            m_logmutex.unlock();
        }
    }
    {
        // 获取当前时间戳
        auto now = std::chrono::system_clock::now();
        
        // 将时间戳转换为微秒级别的时间戳
        auto now_us = std::chrono::time_point_cast<std::chrono::microseconds>(now);
        
        // 获取时间戳的微秒数部分
        auto timestamp = now_us.time_since_epoch().count();
        // std::time_t now = std::time(nullptr);
        // // 将时间戳转换为本地时间
        // std::tm* local_time = std::localtime(&now);
        
        // // 格式化本地时间为字符串
        // char timestamp[100];
        // std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local_time);
        
        {
            m_logmutex.lock();
            m_logFile<<m_gcuId<<"|ConfigGCU_End|"<<timestamp<<std::endl;
            m_logmutex.unlock();
        }
    }
    //配置完成后不做额外操作，需在网页中建立TCP连接
    done_flag = true;
}

void IPBUS_GCU_config::synchronize()
{
    WriteReg(hw, "System.gcu_id", m_BEC_port);
    WriteReg(hw, "System.enable", 0x0); 
    
    for (int tap = 0; tap < TAP_NUM; tap ++)
    {
        WriteReg(hw, "System.tap", tap);
        WriteReg(hw, "System.tap", tap + 128);
        WriteReg(hw, "System.reset", 2);
        WriteReg(hw, "System.reset", 0);
        usleep(10000);
        bert.push_back(ReadReg(hw, "System.errcnt2").value());
        int tapout = ReadReg(hw, "System.tap_o").value();
        //std::cout << "[tap, bert] = [" << tapout << ", " << bert[tap] << "]" << std::endl;
    }
    eye_scan();
    WriteReg(hw, "System.reset", 2);
    WriteReg(hw, "System.reset", 0);
    WriteReg(hw, "System.enable", 0x1f);
    int version = ReadReg(hw, "System.version").value();
    int ecode = ReadReg(hw, "System.errcode").value();

    if (ecode != 0)
    {
        config_succ = false;
        spdlog::error("GCU {:>4}, ecode = {:#x}, version = {:#x}", m_gcuId, ecode, version);
        // spdlog::error("GCU {:>4} BEC {:>5} port {:>2}, ecode = {:#x}", m_gcuId, m_BEC_pos, m_BEC_port, ecode);
    }

    // //threshold value/threshold
	// for(int chn_num=0;chn_num<3;chn_num++)    //channel_num
	// {
	// 	if(!WriteRead_Reg(hw, "GPIO.DataAssembly.threshold" + std::to_string(chn_num + 1), "GPIO.DataAssembly.threshold" + std::to_string(chn_num + 1) + "_rd", 0x3fff))
	//     {
	//     	config_succ = false;
	//     }
	// }
}

void IPBUS_GCU_config::eye_scan()
{
    int i = 0;
    int start0 = 0;  //when tap=0, bert=0,need to calculate with end part
    int start_eye = 0, end_eye = 0;
    int eyewin = 10;
    int central = 0;
    int eyewinnew = 0;
    int flagbegin = 0;
    bert.clear();
    while (i < 120)
    {
        if (bert[i] == 0)
        {
            flagbegin = i;
            if (flagbegin == 0)
                start0 = 1;
            while (bert[i] == 0)
                if (i < 120)
                    i++;
                else break;
            eyewinnew = i - flagbegin;
            if (start0 == 1)
            {
                start_eye = eyewinnew;
                start0 = 0;
            }
            if (eyewin < eyewinnew)
            {
                eyewin = eyewinnew;
                central = (flagbegin + i) / 2;
            }
        }
        else
            i ++;
    }
    if (bert[i] == 0)
    {
        end_eye = eyewinnew;
        eyewinnew = end_eye + start_eye;
    }
    if (eyewin < eyewinnew)
    {
        eyewin = eyewinnew;
        if (start_eye > end_eye)
            central = (start_eye - end_eye) / 2;
        else
            central = 120 - (end_eye - start_eye) / 2;
    }
    // if(central == 0)
    //     spdlog::error("GCU {} eye is too narrow", m_gcuId);
    
    WriteReg(hw, "System.tap", central);
    WriteReg(hw, "System.tap", central + 128);
    int version = ReadReg(hw, "System.version").value();
    if (eyewin < 60)
    {
        config_succ = false;
        spdlog::error("GCU {:>4}, eyewin = {}, central = {}, version={:#x}", m_gcuId, eyewin, central, version);
        // spdlog::error("GCU {:>4} BEC {:>5} port {:>2}, eyewin = {}, central = {}", m_gcuId, m_BEC_pos, m_BEC_port, eyewin, central);
    }
    else
        spdlog::info("GCU {:>4}, eyewin = {}, central = {}, version={:#x}", m_gcuId, eyewin, central, version);
        
}

void IPBUS_GCU_config::start_trigger()
{
    std::cout << "start trigger" <<std::endl;
    //added new format
    std::cout<<"mode:"<<m_trgMode<<std::endl;
    bool start_succ = true;
    {
        // 获取当前时间戳
        auto now = std::chrono::system_clock::now();
        
        // 将时间戳转换为微秒级别的时间戳
        auto now_us = std::chrono::time_point_cast<std::chrono::microseconds>(now);
        
        // 获取时间戳的微秒数部分
        auto timestamp = now_us.time_since_epoch().count();
        {
            m_logmutex.lock();
            m_logFile<<m_gcuId<<"|StartGCU_Start|"<<timestamp<<std::endl;
            m_logmutex.unlock();
        }
    }
    if(m_mode == 0)
    {
        // /*
        // if(m_trgMode == "N")   //modified by zhangsh for clear fifo
        {
            if(!gcu_clear_fifo(hw))
            {
                std::time_t now = std::time(nullptr);
        
                // 将时间戳转换为本地时间
                std::tm* local_time = std::localtime(&now);
                
                // 格式化本地时间为字符串
                char time_str[100];
                std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
                {
                    m_logmutex.lock();
                    m_logFile<<m_gcuId<<"|GCU_CLEAR_FIFO_ERROR|"<<time_str<<std::endl;
                    m_redis->set("TCP_Alert", "1");
                    m_logmutex.unlock();
                }
                start_succ = false;
            }
        }

        // hw->getNode("System").getNode("reset").write(0x01);
        // hw->dispatch();
        // hw->getNode("System").getNode("reset").write(0x00);
        // hw->dispatch();
        if(m_trgMode != "PM")
        {
            std::cout<<"m_freq:"<<m_freqValue<<std::endl;
            if(!abc_start_acq_ped(hw, m_freqValue, 1, 0))
            {
                std::time_t now = std::time(nullptr);
        
                // 将时间戳转换为本地时间
                std::tm* local_time = std::localtime(&now);
                
                // 格式化本地时间为字符串
                char time_str[100];
                std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
                {
                    m_logmutex.lock();
                    m_logFile<<m_gcuId<<"|ABC_START_ACQ_PED_ERROR|"<<time_str<<std::endl;
                    m_redis->set("TCP_Alert", "1");
                    m_logmutex.unlock();
                }
                start_succ = false;
            }
        }
        else
        {
            if(!abc_start_acq_pm(hw, 1, 0, "Y"))
            {
                std::time_t now = std::time(nullptr);
        
                // 将时间戳转换为本地时间
                std::tm* local_time = std::localtime(&now);
                
                // 格式化本地时间为字符串
                char time_str[100];
                std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
                {
                    m_logmutex.lock();
                    m_logFile<<m_gcuId<<"|ABC_START_ACQ_PM_ERROR|"<<time_str<<std::endl;
                    m_redis->set("TCP_Alert", "1");
                    m_logmutex.unlock();
                }
                start_succ = false;
            }
        }
        // sleep(2);
        if(!gcu_start_acq(hw))
        {
            std::time_t now = std::time(nullptr);
        
            // 将时间戳转换为本地时间
            std::tm* local_time = std::localtime(&now);
            
            // 格式化本地时间为字符串
            char time_str[100];
            std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
            {
                m_logmutex.lock();
                m_logFile<<m_gcuId<<"|GCU_START_ACQ_ERROR|"<<time_str<<std::endl;
                m_redis->set("TCP_Alert", "1");
                m_logmutex.unlock();
            }
            start_succ = false;
        }
        
        
        
        
        
        // hw->getNode("System").getNode("reset").write(0x01);
        // hw->dispatch();
        // hw->getNode("System").getNode("reset").write(0x00);
        // hw->dispatch();
        
        // */
    }
    //开始TCP取数
    
    if(!start_succ)
    {
        std::time_t now = std::time(nullptr);
        // 将时间戳转换为本地时间
        std::tm* local_time = std::localtime(&now);
        
        // 格式化本地时间为字符串
        char time_str[100];
        std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
        {
            m_logmutex.lock();
            m_logFile<<m_gcuId<<"|Start_Failed|"<<time_str<<std::endl;
            m_logmutex.unlock();
        }
    }
    // {
    //     // 获取当前时间戳
    //     auto now = std::chrono::system_clock::now();
        
    //     // 将时间戳转换为微秒级别的时间戳
    //     auto now_us = std::chrono::time_point_cast<std::chrono::microseconds>(now);
        
    //     // 获取时间戳的微秒数部分
    //     auto timestamp = now_us.time_since_epoch().count();
    //     {
    //         m_logmutex.lock();
    //         m_logFile<<m_gcuId<<"|StartGCU_End|"<<timestamp<<std::endl;
    //         m_logmutex.unlock();
    //     }
    // }
    


}

void IPBUS_GCU_config::stop_trigger()
{   
    std::cout << "stop trigger" <<std::endl;
    bool stop_succ = true;
    {
        // 获取当前时间戳
        auto now = std::chrono::system_clock::now();
        
        // 将时间戳转换为微秒级别的时间戳
        auto now_us = std::chrono::time_point_cast<std::chrono::microseconds>(now);
        
        // 获取时间戳的微秒数部分
        auto timestamp = now_us.time_since_epoch().count();
        {
            m_logmutex.lock();
            m_logFile<<m_gcuId<<"|StopGCU_Start|"<<timestamp<<std::endl;
            m_logmutex.unlock();
        }
    }
    if(m_mode == 0)
    {
        // /*
        
        if(!gcu_stop_acq(hw))
        {
            std::time_t now = std::time(nullptr);
        
            // 将时间戳转换为本地时间
            std::tm* local_time = std::localtime(&now);
            
            // 格式化本地时间为字符串
            char time_str[100];
            std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
            {
                m_logmutex.lock();
                m_logFile<<m_gcuId<<"|GCU_STOP_ACQ_ERROR|"<<time_str<<std::endl;
                m_redis->set("TCP_Alert", "1");
                m_logmutex.unlock();
            }
            stop_succ = false;
        }
        if(!abc_stop_acq(hw))
        {
            std::time_t now = std::time(nullptr);
        
            // 将时间戳转换为本地时间
            std::tm* local_time = std::localtime(&now);
            
            // 格式化本地时间为字符串
            char time_str[100];
            std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
            {
                m_logmutex.lock();
                m_logFile<<m_gcuId<<"|ABC_STOP_ACQ_ERROR|"<<time_str<<std::endl;
                m_redis->set("TCP_Alert", "1");
                m_logmutex.unlock();
            }
            stop_succ = false;
        }
        
        if(!gcu_clear_fifo(hw))
        {
            std::time_t now = std::time(nullptr);

            // 将时间戳转换为本地时间
            std::tm* local_time = std::localtime(&now);
            
            // 格式化本地时间为字符串
            char time_str[100];
            std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
            {
                m_logmutex.lock();
                m_logFile<<m_gcuId<<"|GCU_CLEAR_FIFO_ERROR|"<<time_str<<std::endl;
                m_redis->set("TCP_Alert", "1");
                m_logmutex.unlock();
            }
            stop_succ = false;
        }
        if(!abc_clear_fifo(hw))
        {
            std::time_t now = std::time(nullptr);

            // 将时间戳转换为本地时间
            std::tm* local_time = std::localtime(&now);
            
            // 格式化本地时间为字符串
            char time_str[100];
            std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
            {
                m_logmutex.lock();
                m_logFile<<m_gcuId<<"|ABC_CLEAR_FIFO_ERROR|"<<time_str<<std::endl;
                m_redis->set("TCP_Alert", "1");
                m_logmutex.unlock();
            }
            stop_succ = false;
        }
        // */
    }
    //停止TCP取数
    if(!stop_succ)
    {
        std::time_t now = std::time(nullptr);
        // 将时间戳转换为本地时间
        std::tm* local_time = std::localtime(&now);
        
        // 格式化本地时间为字符串
        char time_str[100];
        std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
        {
            m_logmutex.lock();
            m_logFile<<m_gcuId<<"|Stop_Failed|"<<time_str<<std::endl;
            m_logmutex.unlock();
        }
    }
    // {
    //     // 获取当前时间戳
    //     auto now = std::chrono::system_clock::now();
        
    //     // 将时间戳转换为微秒级别的时间戳
    //     auto now_us = std::chrono::time_point_cast<std::chrono::microseconds>(now);
        
    //     // 获取时间戳的微秒数部分
    //     auto timestamp = now_us.time_since_epoch().count();
    //     {
    //         m_logmutex.lock();
    //         m_logFile<<m_gcuId<<"|StopGCU_End|"<<timestamp<<std::endl;
    //         m_logmutex.unlock();
    //     }
    // }
}

void IPBUS_GCU_config::reset()
{
    std::cout << "reset trigger" <<std::endl;
    hw->getNode("System").getNode("reset").write(0x01);
    hw->dispatch();
    hw->getNode("System").getNode("reset").write(0x00);
    hw->dispatch();
    
}


void IPBUS_GCU_config::close_gcu()
{
    delete hw;
}
