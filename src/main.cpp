#include <sys/types.h>
#include <string>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <algorithm>
#include <vector>
#include <regex>
#include <IPBUS_GCU_config.h>


void Stringsplit(const std::string& str, const std::string& split, std::vector<std::string>& res)
{
	//std::regex ws_re("\\s+"); // 正则表达式,匹配空格 
	std::regex reg(split);		// 匹配split
	std::sregex_token_iterator pos(str.begin(), str.end(), reg, -1);
	decltype(pos) end;              // 自动推导类型 
	for (; pos != end; ++pos)
	{
		res.push_back(pos->str());
	}
}

void usage()
{
    std::cout << 
    "./Config_SPMT\n\
    参数：\n\
    -g 单GCU模式，需加上GCU编号，且该GCU必须在配置文件内，不写默认读配置文件所有GCU\n\
    -c start：开始取数  stop：结束取数  config：结束+配置\n\
    -m 1:模拟源\n\
    " << std::endl;
}

int main(int argc, char* argv[])
{
    int opt;
    std::string single_gcu = "No", cmd_flag = "No";
    int gcu_id = 0;
    int mode = 0;
    std::string cmd = "default";

    while((opt = getopt(argc, argv, "g:c:m:ho:")) != -1)
    {
        switch(opt)
        {
            case 'g':
                single_gcu = "Yes";
                gcu_id = std::stoi(std::string(optarg));
                break;
            case 'c':
                cmd_flag = "Yes";
                cmd = std::string(optarg);
                break;
            case 'm':
                mode = std::stoi(std::string(optarg));
                break;
            case 'h':
                usage();
                return 0;
            default:
                std::cout << "Unknown option!" <<std::endl;
                return -1;
        }
    }
    char* dataPath_env = std::getenv("DATA_PATH");
    std::string dataPath;
    if (dataPath_env == nullptr) {
        std::cerr << "Environment variable DATA_PATH not set." << std::endl;
        dataPath="../single_SPMT_v2/data/";
    }
    else {dataPath = dataPath_env;}
    char* configPath_env = std::getenv("CONFIG_PATH");
    std::string cfg_path;
    if (configPath_env == nullptr) {
        std::cerr << "Environment variable CONFIG_PATH not set." << std::endl;
        cfg_path="cfg/cfg_PM/DAQ_Config.txt";
    }
    else{cfg_path = configPath_env;}
    char* runNumberPath_env = std::getenv("RUNNUMBER_PATH");
    std::string runNumberPath;
    if (runNumberPath_env == nullptr) {
        std::cerr << "Environment variable RUNNUMBER_PATH not set." << std::endl;
        runNumberPath="cfg/DAQ_RunNumber.txt";
    }
    else{runNumberPath = runNumberPath_env;}
    char* htmlPath_env = std::getenv("HTML_PATH");
    std::string htmlPath;
    if (htmlPath_env == nullptr) {
        std::cerr << "Environment variable HTML_PATH not set." << std::endl;
        htmlPath="cfg/htmlConfig.txt";
    }
    else{htmlPath = htmlPath_env;}
    std::cout<<"runNumberPath:"<<runNumberPath<<" dataPath:"<<dataPath<<" configPath:"<<cfg_path<<std::endl;

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

    char* installPath_env = std::getenv("CMAKE_PREFIX_PATH");
    std::string installPath;
    if (installPath_env == nullptr) {
        std::cerr << "Environment variable IP_PORT not set." << std::endl;
        // return 1;
        installPath="~/external/installed";
    }
    else{installPath = installPath_env;}
    
    // std::string cfg_path = "cfg/cfg_PM/DAQ_Config.txt";
    // std::string runNumberPath = "cfg/DAQ_RunNumber.txt";  //该文件保存当前run号
    // std::string dataPath = "../single_SPMT_v3/data/";    //修改dataPath路径
    // std::string dataPath = "/home/run/JUNO_data/SPMT_data/SPMT_lightoff_240526/";
    // std::string htmlPath = "cfg/htmlConfig.txt";

    // std::ofstream outFile;
    std::ofstream outFile(htmlPath, std::ios::out);
    if(!outFile.is_open())
    {
        std::cout<<"Write html Config file error"<<std::endl;
    }

    std::vector<std::map<std::string, std::string>> gcu_list;
    std::vector<std::shared_ptr<gcu::IPBUS_GCU_config>> config_workers;
    std::fstream config_file;
    config_file.open(cfg_path,std::ios::in);
    std::string line;
    int row_count = 0;
    std::getline(config_file, line);
    bool flag_avail = false;
    while (std::getline(config_file, line))
    {
        std::vector<std::string> row;
        Stringsplit(line, " ", row);
        if (row_count % CAT_NUM == 0)
        {
            flag_avail = false;
            if(row[7] == "Y")
            {
                std::map<std::string, std::string> gcu;
                gcu["GCU_ID"] = row[0];
                gcu["Thres_HG" + std::to_string(row_count % CAT_NUM)] = row[2];
                gcu["Thre" + std::to_string(row_count % CAT_NUM)] = row[3];
                gcu["FreqValue"] = row[4];
                gcu["TrigMode_PM"]= row[5];
                gcu["DisTrigger"]= row[6];
                gcu["IP"]= row[8];
                gcu["BEC_Port"] = row[11];
                if(mode == 0)
                {
                    gcu["GCU_Auto"+std::to_string(row_count % CAT_NUM)] = row[9];
                    gcu["CAT_Config"+std::to_string(row_count % CAT_NUM)] = row[10];
                }
                else
                {
                    gcu["GCU_Auto"+std::to_string(row_count % CAT_NUM)] = "Y";
                    gcu["CAT_Config"+std::to_string(row_count % CAT_NUM)] = "Y";
                }
                gcu_list.push_back(gcu);
                flag_avail = true;
                
            }
        }
        else if(flag_avail)
        {
            gcu_list.back()["Thres_HG" + std::to_string(row_count % CAT_NUM)] = row[2];
            gcu_list.back()["Thre" + std::to_string(row_count % CAT_NUM)] = row[3];
            if(mode == 0)
            {
                gcu_list.back()["GCU_Auto" + std::to_string(row_count % CAT_NUM)] = row[9];
                gcu_list.back()["CAT_Config" + std::to_string(row_count % CAT_NUM)] = row[10];
            }  
            else
            {
                gcu_list.back()["GCU_Auto" + std::to_string(row_count % CAT_NUM)] = "Y";
                gcu_list.back()["CAT_Config" + std::to_string(row_count % CAT_NUM)] = "Y";
            }
                
        }
        row_count++;
    }
    config_file.close();
    
    std::time_t startRun_time, stopRun_time;
    int runNumber = 0;
    std::ifstream infile(runNumberPath);

    if (infile.is_open()) {
        infile>>runNumber;
        infile.close();
        std::cout<<"runNumber:"<<runNumber<<std::endl;
    } else {
        std::cerr << "Failed to open runNumber file: " << runNumberPath << std::endl;
    }    
    
    std::string logDirname;
    
    if(cmd != "stop")
    {
        std::string command;
        command = installPath + "/bin/redis-cli -h "+redisIp+" -p "+redisPort+" set TCP_Alert 0";
        system(command.c_str());
        //拿到run号，保存配置文件,拿到log文件名
        std::string dirName = dataPath + std::to_string(runNumber);
        command = "mkdir -p " + dirName;
        system(command.c_str());
        command = "cp " + cfg_path + " " + dirName;
        system(command.c_str());

        command = "mv " + dirName + "/DAQ_Config.txt " + dirName + "/DAQ_Config_"+std::to_string(runNumber)+".txt";
        system(command.c_str());
        
        // logDirname = dirName + "/Log/"; //old
        logDirname = "../Log/"+ std::to_string(runNumber) + "/Log/";
        command = "mkdir -p "+ logDirname;
        // std::cout<<"command:"<<dirName<<' '<<command<<std::endl;
        system(command.c_str());

        // if(cmd != "start")
        // {
        //     command = "rm -rf " + logDirname + "*";
        //     std::cout<<"command:"<<command<<std::endl;
        //     system(command.c_str());
        // }
        
        logDirname = logDirname+"log"+std::to_string(runNumber)+"_";

        if(cmd == "start")  //更新run号
        {
            std::ofstream out_file(runNumberPath, std::ios::trunc); // 打开文件覆盖模式
            if (out_file.is_open()) {
                int nextRun = runNumber+1;
                out_file<<nextRun<<std::endl;
                startRun_time = std::time(nullptr);
                out_file<<startRun_time<<std::endl;
                std::cout<<"runNUmberPath:"<<runNumber<<" "<<startRun_time<<std::endl;
                out_file.close();
            } else {
                std::cerr << "Failed to update runNumber file: " << runNumberPath << std::endl;
            }
        }
        
    }
    else
    {
        logDirname = "../Log/" + std::to_string(runNumber-1) + "/Log/log"+std::to_string(runNumber-1)+"_";
        std::string command;
        command = installPath + "/bin/redis-cli -h "+redisIp+" -p "+redisPort+" set TCP_Stop 0";
        system(command.c_str());
    }
    

    for (auto gcu : gcu_list)
    {
        if (single_gcu == "Yes")
        {
            if (std::stoi(gcu["GCU_ID"]) == gcu_id)
            {
                config_workers.push_back(std::make_shared<gcu::IPBUS_GCU_config>(gcu, logDirname, mode));
                outFile<<gcu_id<<std::endl;
                break;
            }
        }
        else
        {
            config_workers.push_back(std::make_shared<gcu::IPBUS_GCU_config>(gcu, logDirname, mode));
            outFile<<std::stoi(gcu["GCU_ID"])<<std::endl;
        }
            
    }
    outFile.close();
    
    if(cmd == "config")
    {
        std::string command;
        command = installPath + "/bin/redis-cli -h "+redisIp+" -p "+redisPort+" set TCP_Config 0";
        system(command.c_str());
    }
    for (auto config : config_workers)
        config->start(cmd);

//    while (std::cin.get() != 's');
    if(cmd == "config")
    {
        bool config_flag = true;
        for (auto config : config_workers)
        {
            while(1)
            {
                if(config->get_flag() != -1)
                {
                    break;
                }
                usleep(10000);
            }
            if(!config->get_flag())
                config_flag = false;
        }
        if(config_flag)
        {
            std::string command;
            command = installPath + "/bin/redis-cli -h "+redisIp+" -p "+redisPort+" set TCP_Config 1";
            system(command.c_str());
        }
    }
    
    

    for (auto config : config_workers)
        config->stop(cmd);

    std::cout << "exit" << std::endl;
    if(cmd == "start")
    {
        //start置1,清空redis，monitor置1
        // std::string command = "/opt/daq/daqv2.0/external/installed/bin/redis-cli -h hd101 -p 8088 --scan --pattern \"ChanNhit*\"  | xargs /opt/daq/daqv2.0/external/installed/bin/redis-cli -h hd101 -p 8088 del";
        // system(command.c_str());
        std::string command;
        command = installPath + "/bin/redis-cli -h "+redisIp+" -p "+redisPort+" set TCP_Start 1";
        system(command.c_str());
        command = installPath + "/bin/redis-cli -h "+redisIp+" -p "+redisPort+" --scan --pattern \"JUNO_SPMT*\"  | xargs "+installPath + "/bin/redis-cli -h "+redisIp+" -p "+redisPort+" del";
        system(command.c_str());
        command = installPath + "/bin/redis-cli -h "+redisIp+" -p "+redisPort+" set TCP_Monitor 1";
        system(command.c_str());
        // command = installPath + "/bin/redis-cli -h "+redisIp+" -p "+redisPort+" set DAQ_READY 0";
        // system(command.c_str());
    }
    else
    {
        std::string command;
        command = installPath + "/bin/redis-cli -h "+redisIp+" -p "+redisPort+" set TCP_Start 0";
        system(command.c_str());
        command = installPath + "/bin/redis-cli -h "+redisIp+" -p "+redisPort+" set TCP_Monitor 0";
        system(command.c_str());

        if(cmd == "stop")
        {
            std::ofstream outfile(runNumberPath, std::ios_base::app); // 打开文件追加模式
            if (outfile.is_open()) {
                time_t stopRun = time(nullptr); // 获取当前时间戳
                outfile << stopRun << std::endl; // 写入文件
                outfile.close(); // 关闭文件
            } else {
                std::cerr << "Failed to write RunNumber file for stopTime: " << runNumberPath << std::endl;
            }
        }
        
    }
    
    return 1;
}