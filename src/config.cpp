#include <vector>
#include <string>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include "config.h"
// #include "include/global.h"

using namespace uhal;

bool gcu_start_acq(HwInterface* hw)
{
    int times = 10;
    while(times>0)
    {
        try
        {
            hw->getNode("ipbus_fifo_0.ctrl").read();
            hw->dispatch();
            hw->dispatch();
            // std::cout<<"IPBUS Connection OK"<<std::endl;
        }
        catch(std::exception &e)
        {
            std::cout<<e.what()<<std::endl;
            times--;
            if(times == 0)
                return false;
            continue;
        }
        break;
    }
    
    
    bool if_success = false;
    times = 10;
    while(times > 0)
    {
        try
        {
            ValWord<uint32_t> gpioValue = hw->getNode("GPIO").read();
            hw->dispatch();
            gpioValue = gpioValue | 0x02;
            hw->getNode("GPIO").write(gpioValue);
            hw->dispatch();
            if_success = true;
        }
        catch(std::exception &e)
        {
            std::cout<<"GCU START ACQ ERR"<<std::endl;
            times--;
            if(times == 0)
                return false;
            continue;
            // exit(-1);
        }
        // std::cout<<"GCU Acquisiton Start"<<std::endl;
        break;
    }
    
    return if_success;
}

bool gcu_stop_acq(HwInterface* hw)
{
    int times = 10;
    while(times>0)
    {
        try
        {
            hw->getNode("ipbus_fifo_0.ctrl").read();
            hw->dispatch();
            hw->dispatch();
            // std::cout<<"IPBUS Connection OK"<<std::endl;
        }
        catch(std::exception &e)
        {
            std::cout<<e.what()<<std::endl;
            times--;
            if(times == 0)
            {
                return false;
            }
            continue;
        }
        break;
    }
    times = 10;
    while(times>0)
    {
        try
        {
            ValWord<uint32_t> gpioValue = hw->getNode("GPIO").read();
            hw->dispatch();
            gpioValue = gpioValue & 0xFFFFFFFD;
            hw->getNode("GPIO").write(gpioValue);
            hw->dispatch();

            // std::cout<<"GCU Acquisition Stop"<<std::endl;
            return true;
        }
        catch(std::exception &e)
        {
            std::cout<<"GCU STOP ACQ ERR"<<std::endl;
            times--;
            if(times == 0)
            {
                return false;
            }
            // exit(-1);
        }
    }
        

}

void abc_master_reset(HwInterface* hw)
{
    spi_send(hw, 0x10,0x00,0x00,0x00);
    sleep(0.21);
    spi_send(hw, 0x07,0x00,0x00,0x00);
    std::cout<<"ABC Master Reset"<<std::endl;
}

//byte3设置阈值，唯一区别和PM取数方式
bool abc_start_acq_ped(HwInterface* hw, int freqNum, int charge, int dds)
{
    int times = 10;
    while(times > 0)
    {
        try
        {
            hw->getNode("ipbus_fifo_0.ctrl").read();
            hw->dispatch();
            hw->dispatch();
            // std::cout<<"IPBUS Connection OK"<<std::endl;
            
        }
        catch(std::exception &e)
        {
            std::cout<<e.what()<<std::endl;
            times--;
            if(times == 0)
                continue;
        }
        break;
    }
    
    
    int trigger = 0;
    uint32_t Feq = uint32_t(double(1.0/double(freqNum*25))*1000000);
    uint32_t Freq = (Feq-0x40) >> 8;
    Freq = Freq & 0xFF;
    uint32_t byte_3 = 0x01;
    // uint32_t byte_2 = charge + (dds<<1);
    // uint32_t byte_3 = 0x01;
    // std::map<std::string, int> freqValues;
    // freqValues["0,630KHz"]     =0xF0;
    // freqValues["0,674KHz"]     =0xE0;
    // freqValues["0,724KHz"]     =0xD0;
    // freqValues["0,781KHz"]     =0xC0;
    // freqValues["0,850KHz"]     =0xB0;
    // freqValues["0,931KHz"]     =0xA0;
    // freqValues["1,029KHz"]     =0x90;
    // freqValues["1,150KHz"]     =0x80;
    // freqValues["1,304KHz"]     =0x70;
    // freqValues["1,505KHz"]     =0x60;
    // freqValues["1,779KHz"]     =0x50;
    // freqValues["2,175KHz"]     =0x40;
    // freqValues["2,799KHz"]     =0x30;
    // freqValues["3,924KHz"]     =0x20;
    // freqValues["6,561KHz"]     =0x10;
    // freqValues["20KHz"]        =0x00;
    // uint32_t byte_3 = 0xF1;
    uint32_t byte_2 = Freq;
    
    // std::cout<<"freqNum:"<<freqNum<<std::endl;
    // std::cout<<"freqVaL:"<<freqValues[freqNum]<<std::endl;
    // std::cout<<"byte2:"<<std::hex<<byte_2<<' '<<"byte_3:"<<byte_3<<std::endl;
    // exit(-1);

    spi_send(hw, 0x06, 0xFF, byte_2, byte_3);
    sleep(0.21);
    spi_send(hw, 0x06, 0xFF, byte_2, byte_3);
    
    // std::cout<<"ABC Acquisition Started"<<std::endl;
    return true;
}

bool abc_start_acq_pm(HwInterface* hw, int charge, int dds, std::string highrateMode)
{
    int times = 10;
    while(times>0)
    {
        try
        {
            hw->getNode("ipbus_fifo_0.ctrl").read();
            hw->dispatch();
            hw->dispatch();
            // std::cout<<"IPBUS Connection OK"<<std::endl;
        }
        catch(std::exception &e)
        {
            std::cout<<e.what()<<std::endl;
            times--;
            if(times == 0)
                return false;
            continue;
        }
        break;
    }
    
    
    
    uint32_t byte_2 = charge + (dds<<1);

    uint32_t byte_3;
    if(highrateMode == "Y")
        byte_3 = 0xF2;
    else
        byte_3 = 0xF0;

    spi_send(hw, 0x06, 0xFF, byte_2, byte_3);
    sleep(0.21);
    if(!spi_send(hw, 0x06, 0xFF, byte_2, byte_3))
    {
        return false;
    }

    // std::cout<<"ABC Acquisition Started"<<std::endl;
    return true;
}

bool abc_stop_acq(HwInterface* hw)
{
    int times = 10;
    while(times>0)
    {
        try
        {
            hw->getNode("ipbus_fifo_0.ctrl").read();
            hw->dispatch();
            hw->dispatch();
            // std::cout<<"IPBUS Connection OK"<<std::endl;
        }
        catch(std::exception &e)
        {
            std::cout<<e.what()<<std::endl;
            times--;
            if(times == 0)
            {
                std::cout<<"ABC_STOP_ACQ_ERROR"<<std::endl;
                return false;
            }
                
            continue;
        }
        break;
    }
    
  
    spi_send(hw, 0x07,0x00,0x00,0x00);
    sleep(0.21);
    bool if_success = spi_send(hw, 0x07,0x00,0x00,0x00);
    if(if_success)
    {
        std::cout<<"ABC Acquisition Stopped"<<std::endl;
        return true;
    }
    else
    {
        std::cout<<"ABC_STOP_ACQ_ERROR"<<std::endl;
        return false;
    }
    
}

bool gcu_clear_fifo(HwInterface* hw)
{
    int times = 10;
    while(times > 0)
    {
        try
        {
            hw->getNode("ipbus_fifo_0.ctrl").read();
            hw->dispatch();
            hw->dispatch();
            // std::cout<<"IPBUS Connection OK"<<std::endl;
        }
        catch(std::exception &e)
        {
            std::cout<<e.what()<<std::endl;
            times--;
            if(times == 0)
                return false;
            continue;
        }
        break;
    }
    

    // abc_stop_acq(hw);
    if(!spi_send(hw, 0x09, 0x00, 0x00, 0x00))
        return false;
    sleep(0.21);
    ValWord<uint32_t> count = readCounter(hw);
    
    // std::cout<<"count:"<<count.value()<<std::endl;
    
    while(int(count) != 0)
    {
        ValVector<uint32_t> fifo;
        fifo = readFifo(hw, int(count));
        count = readCounter(hw);
        // std::cout<<"count:"<<count.value()<<std::endl;
    }

    // std::cout<<"GCU FIFO CLEAR SUCCESS"<<std::endl;

    if(!spi_send(hw, 0x10, 0x00, 0x00, 0x00))
        return false;
    sleep(0.21);
    if(!spi_send(hw, 0x07, 0x00, 0x00, 0x00))
        return false;
    // std::cout<<"ABC FIFO CLEARED"<<std::endl;
    return true;
}

bool abc_clear_fifo(HwInterface* hw) 
{
    int times = 10;
    while(times > 0)
    {
        try
        {
            hw->getNode("ipbus_fifo_0.ctrl").read();
            hw->dispatch();
            hw->dispatch();
            // std::cout<<"IPBUS Connection OK"<<std::endl;
        }
        catch(std::exception &e)
        {
            std::cout<<e.what()<<std::endl;
            times--;
            if(times == 0)
                return false;
            continue;
        }
        break;
    }
     
    if(!spi_send(hw, 0x10, 0x00, 0x00, 0x00))
        return false;
    sleep(0.21);
    if(!spi_send(hw, 0x07, 0x00, 0x00, 0x00))
        return false;
    return true;

}

bool gcu_wait_for_event(HwInterface* hw, int waitTime) 
{

    try
    {
        hw->getNode("ipbus_fifo_0.ctrl").read();
        hw->dispatch();
        // std::cout<<"IPBUS Connection OK"<<std::endl;
    }
    catch(std::exception &e)
    {
        std::cout<<e.what()<<std::endl;
    }
    try
    {
        hw->dispatch();
    }
    catch(std::exception &e)
    {
        std::cout<<e.what()<<std::endl;
    }
    bool if_timeOut = false;
    bool flag_gcu = false;

    time_t timeOut;
    
    timeOut = time(NULL) + waitTime;
    try
    {
        while(flag_gcu == false)
        {
            ValWord<uint32_t> gpioValue = hw->getNode("GPIO").read();
            hw->dispatch();
            uint32_t value = gpioValue & 0x8;
            if(value == 0x8)
            {
                flag_gcu = true;
                if_timeOut = false;
            }
            else if(time(NULL) > timeOut)
            {
                flag_gcu = true;
                if_timeOut = true;
            }
            // std::cout<<"WAIT"<<std::endl;
        }
    }
    catch(std::exception &e)
    {
        std::cout<<"GCU WAITING FOR EVENT ERR"<<std::endl;
        return true;
        // exit(-1);
    }
    // std::cout<<"GCU STARTED TO RECEIVE"<<std::endl;
    return if_timeOut;
}


void abc_init(HwInterface* hw)
{
    try
    {
        hw->getNode("ipbus_fifo_0.ctrl").read();
        hw->dispatch();
        // std::cout<<"IPBUS Connection OK"<<std::endl;
    }
    catch(std::exception &e)
    {
        std::cout<<e.what()<<std::endl;
    }
    try
    {
        hw->dispatch();
        // std::cout<<"IPBUS Connection OK"<<std::endl;
    }
    catch(std::exception &e)
    {
        std::cout<<e.what()<<std::endl;
    }
    hw->getNode("SPI_Master_Controller").getNode("ss").write(0xFF);
    hw->dispatch();
    hw->getNode("SPI_Master_Controller").getNode("ctrl").write(0x2920);
    hw->getNode("SPI_Master_Controller").getNode("ctrl").write(0x2820);
    hw->getNode("SPI_Master_Controller").getNode("ctrl").read();
    hw->dispatch();
}

void gcu_init(HwInterface* hw)
{
    try
    {
        hw->getNode("ipbus_fifo_0.ctrl").read();
        hw->dispatch();
        // std::cout<<"IPBUS Connection OK"<<std::endl;
    }
    catch(std::exception &e)
    {
        std::cout<<e.what()<<std::endl;
    }
    try
    {
        hw->dispatch();
        // std::cout<<"IPBUS Connection OK"<<std::endl;
    }
    catch(std::exception &e)
    {
        std::cout<<e.what()<<std::endl;
    }
}

void hv_init(HwInterface* hw)
{
    try
    {
        hw->getNode("uart_0.setup").read();
        hw->dispatch();
        // std::cout<<"IPBUS Connection OK"<<std::endl;
    }
    catch(std::exception &e)
    {
        std::cout<<e.what()<<std::endl;
    }

}

ValWord<uint32_t> readCounter(HwInterface* hw)
{
    int times = 10;
    while(times > 0)
    {
        try
        {
            hw->getNode("ipbus_fifo_0.ctrl").read();
            hw->dispatch();
            // std::cout<<"IPBUS Connection OK"<<std::endl;
        }
        catch(std::exception &e)
        {
            std::cout<<e.what()<<std::endl;
            times--;
            if(times == 0)
             return false;
            // usleep(10000);
            continue;
        }
        break;
    }
    
    times = 10;
    while(times>0)
    {
        try
        {
            ValWord<uint32_t> count;
            count =  hw->getNode("ipbus_fifo_0.status.count").read();
            hw->dispatch();
            // std::cout<<"GCU_COUNT="<<count<<std::endl;
            return count;
        }
        catch(std::exception &e)
        {
            std::cout<<"Read COUNT ERR"<<std::endl;
            times--;
            if(times == 0)
                return false;
            // usleep(10000);
            continue;
            // exit(-1);
        }
        
    }
    
}

ValVector<uint32_t> readFifo(HwInterface* hw, uint32_t count)
{
    int times = 10;
    while(times > 0)
    {
        try
        {
            ValVector<uint32_t> fifo = hw->getNode("ipbus_fifo_0.data").readBlock(count);
            hw->dispatch();
            return fifo;
        }
        catch(std::exception &e)
        {
            std::cout<<"READ FIFO ERR"<<std::endl;
            std::cout<<e.what()<<std::endl;
            times--;
            if(times == 0)
                return false;
            // exit(-1);
        }
    }
    
    
}

//********SPI协议************
bool spi_send(HwInterface* hw, uint32_t byte_0, uint32_t byte_1, uint32_t byte_2, uint32_t byte_3)
{
    uint32_t spiD0_value = byte_3 + (byte_2 << 8) + (byte_1 << 16) +(byte_0 << 24);
    // std::cout<<"spi_value:"<<hex<<spiD0_value<<std::endl;

    int times = 10;
    while(times>0)
    {
        try{
            hw->getNode("SPI_Master_Controller").getNode("d0").write(spiD0_value);
            hw->dispatch();

            ValWord<uint32_t> spiCtrl_value = hw->getNode("SPI_Master_Controller").getNode("ctrl").read();
            hw->dispatch();
            // std::cout<<"spiCtrl_value:"<<spiCtrl_value<<std::endl;

            spiCtrl_value = spiCtrl_value | 0x100;
            hw->getNode("SPI_Master_Controller").getNode("ctrl").write(spiCtrl_value);
            // std::cout<<"spiCtrl_value:"<<spiCtrl_value<<std::endl;
            hw->dispatch();

            hw->getNode("SPI_Master_Controller").getNode("ctrl").write(spiCtrl_value);
            hw->dispatch();
        }
        catch(std::exception &e)
        {
            times--;
            std::cout<<"times:"<<times<<" SPI_SEND COMMAND ERROR"<<std::endl;
            std::cout<<e.what()<<std::endl;
            if(times == 0)
                return false;
            continue;
        }
        return true;
        
    }

    

}

bool spi_send_threshold(HwInterface* hw, uint32_t byte_0, uint32_t byte_1, uint32_t byte_2, uint32_t byte_3)
{
    uint32_t spiD0_value = 0;
    spiD0_value = byte_3 + (byte_2 << 8) + (byte_1 << 16) +(byte_0 << 24);
    // std::cout<<"spi_value:"<<hex<<spiD0_value<<std::endl;
    int times = 10;
    while(times--)
    {
        try{
            hw->getNode("SPI_Master_Controller").getNode("d0").write(spiD0_value);
            hw->dispatch();

            ValWord<uint32_t> spiCtrl_value = hw->getNode("SPI_Master_Controller").getNode("ctrl").read();
            hw->dispatch();
            // std::cout<<"spiCtrl_value:"<<spiCtrl_value<<std::endl;

            spiCtrl_value = spiCtrl_value | 0x100;
            hw->getNode("SPI_Master_Controller").getNode("ctrl").write(spiCtrl_value);
            // std::cout<<"spiCtrl_value:"<<spiCtrl_value<<std::endl;
            hw->dispatch();

            hw->getNode("SPI_Master_Controller").getNode("ctrl").write(spiCtrl_value);
            hw->dispatch();
        }
        catch(std::exception &e)
        {
            times--;
            std::cout<<"SPI_SEND_THRESHOLD COMMAND ERROR"<<std::endl;
            std::cout<<e.what()<<std::endl;
            if(times == 0)
                return false;
            continue;
        }
        return true;
        
    }

    
    
}

ValWord<uint32_t> spi_read(HwInterface* hw, uint32_t byte_0, uint32_t byte_1, uint32_t byte_2, uint32_t byte_3)
{
    uint32_t val ;
    ValWord<uint32_t> reg ;
    val = byte_3 + (byte_2<<8)+(byte_1<<16)+(byte_0<<24);
    hw->getNode("SPI_Master_Controller").getNode("d0").write(val);
    hw->dispatch();

    reg = hw->getNode("SPI_Master_Controller").getNode("ctrl").read();
    hw->dispatch();

    reg = reg | 0x100;
    hw->getNode("SPI_Master_Controller").getNode("ctrl").write(reg);
    hw->dispatch();
    hw->getNode("SPI_Master_Controller").getNode("ctrl").write(reg);
    hw->dispatch();

    reg = hw->getNode("SPI_Master_Controller").getNode("d0").read();
    hw->dispatch();
    reg = ((reg>>1)|(reg<<31)) & ((1<<32)-1);
    return reg;
}

bool get_comparaison_status(HwInterface* hw, uint32_t Asic_id)
{
    bool flag = false;
    ValWord<uint32_t> reg = spi_read(hw, 0xC8, 0x00, 0x00, 0x00);
    uint32_t val0 = (uint32_t)reg & 0xFF;
    val0 = val0 >> Asic_id;
    if(val0 != 0)
        flag = true;
    else
        flag = false;
    return flag;
}

uint32_t get_triggers_counter(HwInterface* hw)
{
    ValWord<uint32_t> reg = spi_read(hw, 0xC8, 0x00, 0x00, 0x00);
    uint32_t val1 = ((uint32_t)reg >> 8) & 0xFFF;
    return val1;
}

uint32_t* set_parameters_catirocs(HwInterface* hw, uint32_t Asic_id, std::string disTrigger, std::string gcu_auto, std::string trigMode, std::string asic_flag, int *threhold_time, int *threhold_HGLG)
{
    // std::cout<<"Catiroc #"<<Asic_id<<std::endl;
    ///home/shuihan/zhangsh/SPMT_Readout_zhangsh/config/ThreholdConfig.txt
    static uint32_t val[41] = {0xd7, 0x85, 0xb5, 0x54, 0x41, 0x4a, 0xda, 0xaa, 0xaa, 0xaa, 
                               0xa0, 0x00, 0x07, 0x84, 0x55, 0x2d, 0x91, 0x2a, 0x55, 0xc1, 
                               0x30, 0x4c, 0x13, 0x04, 0xc1, 0x30, 0x4c, 0x13, 0x04, 0xc1, 
                               0x30, 0x4c, 0x13, 0x04, 0xc1, 0x30, 0x4c, 0x13, 0x04, 0x00, 0x00};
    uint32_t Command = 0xCA;
    uint32_t reg_id = 0;
    // char row[1000];


    //**********调量程阈值*********
    val[3] = threhold_HGLG[0];
    val[4] = threhold_HGLG[1];

    //**********调时间阈值**********
    val[5] = threhold_time[0];
    val[6] = threhold_time[1];

    //有改动
    if(disTrigger == "Y")
        val[0] = 0xdf;
    else
        val[0] = 0xd7;
    if(trigMode == "LG")
    {
        val[15] = 0x29;
        val[6] = (val[6] & 0xf0)|0x05;
        
        // val[6] = 0x15;
        val[7] = 0x55;
        val[8] = 0x55;
        val[9] = 0x55;
        val[10] = 0x50;
    }
    // if(gcu_auto != "Y")
    {
        // int disable_channel = std::stoi(gcu_auto);
        uint16_t disable_channel = std::stoi(gcu_auto, nullptr, 16);
        uint16_t temp_value = disable_channel;
        {
            // for (int i = 0; i < 16; ++i) {  // 16 位循环
            //     // 将原数的最低位移到新的反转数的最高位
            //     temp_value <<= 1;              // 左移逆序数
            //     temp_value |= (disable_channel & 1);       // 添加原数的最低位
            //     disable_channel >>= 1;                   // 右移原数，去掉已经处理的最低位
            // }
            val[10] = (val[10]&0xf0) | (uint8_t)(temp_value>>12 & 0xff);
            val[11] = (uint8_t)(temp_value>>4 & 0xff);
            // val[12] = 0x07 | (uint8_t)(temp_value & 0xff);
            val[12] = 0x07 | (uint8_t)((temp_value & 0x0f)<<4);
        }
        // if(Asic_id == 4)
        // {
        //     if(disable_channel == 72)
        //         val[11] = 0x10;
        //     else if(disable_channel == 78)
        //         val[10] = 0xa4;
        // }
        
        // if(disable_channel/16 == Asic_id)
        // {
        //     int temp_channel  = disable_channel%16;
        //     uint16_t temp_value = 1<<temp_channel;
        //     val[10] = 0xa0 | (uint8_t)(temp_value>>12 & 0xff);
        //     val[11] = (uint8_t)(temp_value>>4 & 0xff);
        //     val[12] = 0x07 | (uint8_t)(temp_value & 0xff); 
        //     // std::cout<<"val:"<<(temp_value>>12& 0xff)<<" "<<val[11]<<" "<<(temp_value & 0xff)<<std::endl;
            
        // }
    }
    // std::cout<<"asic_flag："<<asic_flag<<" "<<std::hex<<val[10]<<std::endl;
    if(asic_flag == "N")
    {
        //   0,   1,    2,    3,    4,    5,    6,    7,    8,    9
        // {0xd7, 0x85, 0xb5, 0x54, 0x41, 0x4a, 0xda, 0xaa, 0xaa, 0xaa, 
        //  0xa0, 0x00, 0x07, 0x84, 0x55, 0x2d, 0x91, 0x2a, 0x55, 0xc1, 
        //  0x30, 0x4c, 0x13, 0x04, 0xc1, 0x30, 0x4c, 0x13, 0x04, 0xc1, 
        //  0x30, 0x4c, 0x13, 0x04, 0xc1, 0x30, 0x4c, 0x13, 0x04, 0x00, 0x00}
        val[0] = 0xc0;
        val[2] = 0xa1;
        val[3] = 0x10;
        val[6] = 0xca;
        val[13] = 0x80;
        val[14] = 0x00;
        if(trigMode == "LG")
            val[15] = 0x00;
        else
            val[15] = 0x04;
        val[17] = 0x22;
        val[18] = 0x40;
        uint16_t temp_value = 0xffff;
        {
            val[10] = 0xa0 | (uint8_t)(temp_value>>12 & 0xff);
            val[11] = (uint8_t)(temp_value>>4 & 0xff);
            // val[12] = 0x07 | (uint8_t)(temp_value & 0xff);
            val[12] = 0x07 | (uint8_t)((temp_value & 0x0f)<<4);
        }
    }
    // if(Asic_id == 2)
    // {
    //     val[18] = 0x40;
    //     val[10] = 0xaf;
    //     val[11] = 0xff;
    //     val[12] = 0xf7;
    // }
    // std::cout<<"val[34]:"<<std::hex<<val[3]<<' '<<std::hex<<val[4]<<std::endl;
    // std::cout<<"val[56]:"<<std::hex<<val[5]<<' '<<std::hex<<val[6]<<std::endl;
    // std::cout<<"val:"<<std::hex<<val[6]<<" "<<val[10]<<std::endl;
    for(int i=0; i<41; i++)
    {
        
    
        // std::cout<<"val:"<<std::hex<<val[i]<<' ';
        
        spi_send_threshold(hw, Command, Asic_id, reg_id, val[i]);
        reg_id++;
    }
    // std::cout<<"reg_id:"<<reg_id<<std::endl;
    return val;
}

//added return value
void send_parameters_catirocs(HwInterface* hw, uint32_t Asic_id, std::string disTrigger, std::string gcu_auto, std::string trigMode, std::string asic_flag, int *threhold_time, int *threhold_HGLG)
{
    // std::cout<<"Catiroc #"<<Asic_id<<"SCFile"<<SCFile<<std::endl;
    ///home/shuihan/zhangsh/SPMT_Readout_zhangsh/config/ThreholdConfig.txt
    
    uint32_t val[41] = {0xd7, 0x85, 0xb5, 0x54, 0x00, 0x00, 0x1a, 0xaa, 0xaa, 0xaa, 0xa0, 0x00, 0x07, 0x84, 0x55, 0x2d, 0x91, 0x2a, 0x55, 0xc1, 0x30, 0x4c, 0x13, 0x04, 0xc1, 0x30, 0x4c, 0x13, 0x04, 0xc1, 0x30, 0x4c, 0x13, 0x04, 0xc1, 0x30, 0x4c, 0x13, 0x04, 0x00, 0x00};
    uint32_t Command = 0xCA;
    uint32_t reg_id = 0;
    // char row[1000];

    //**********调量程阈值*********
    val[3] = threhold_HGLG[0];
    val[4] = threhold_HGLG[1];
    
    //**********调时间阈值*************
    val[5] = threhold_time[0];
    val[6] = threhold_time[1];
    // std::cout<<"val"<<val[5]<<std::endl;
    if(disTrigger == "Y")
        val[0] = 0xdf;
    else
        val[0] = 0xd7;
    
    uint16_t disable_channel = std::stoi(gcu_auto, nullptr, 16);
    std::cout<<"trigMode:"<<trigMode<<std::endl;
    if(trigMode == "LG")
    {
        val[15] = 0x29;
        val[6] = (val[6] & 0xf0)|0x05;
        // val[6] = 0x15;
        val[7] = 0x55;
        val[8] = 0x55;
        val[9] = 0x55;
        val[10] = 0x50;
    }
    uint16_t temp_value = disable_channel;
    {
        // for (int i = 0; i < 16; ++i) {  // 16 位循环
        //     // 将原数的最低位移到新的反转数的最高位
        //     temp_value <<= 1;              // 左移逆序数
        //     temp_value |= (disable_channel & 1);       // 添加原数的最低位
        //     disable_channel >>= 1;                   // 右移原数，去掉已经处理的最低位
        // }
        val[10] = (val[10]&0xf0) | (uint8_t)(temp_value>>12 & 0xff);
        val[11] = (uint8_t)(temp_value>>4 & 0xff);
        val[12] = 0x07 | (uint8_t)((temp_value & 0x0f)<<4);
    }
    /*
    if(gcu_auto != "Y")
    {
        // val[15] = 0x29;
        // if(gcu_auto == "LG")
        // {
        //     val[6] = 0x15;
        //     val[7] = 0x55;
        //     val[8] = 0x55;
        //     val[9] = 0x55;
        //     val[10] = 0x50;
        // }
        int disable_channel = std::stoi(gcu_auto);
        // if(Asic_id == 4)
        // {
        //     if(disable_channel == 72)
        //         val[11] = 0x10;
        //     else if(disable_channel == 78)
        //         val[10] = 0xa4;
        // }
        if(disable_channel/16 == Asic_id)
        {
            int temp_channel  = disable_channel%16;
            uint16_t temp_value = 1<<temp_channel;
            val[10] = 0xa0 | (uint8_t)(temp_value>>12 & 0xff);
            val[11] = (uint8_t)(temp_value>>4 & 0xff);
            val[12] = 0x07 | (uint8_t)(temp_value & 0xff); 
            
        }
    }
    */
    // if(Asic_id == 0 || Asic_id==2 || Asic_id==4 ||Asic_id==6 || Asic_id==7)
    // {
    //     val[18] = 0x40;
    //     val[10] = 0xaf;
    //     val[11] = 0xff;
    //     val[12] = 0xf7;
    // }
    if(asic_flag == "N")
    {
        //   0,   1,    2,    3,    4,    5,    6,    7,    8,    9
        // {0xd7, 0x85, 0xb5, 0x54, 0x41, 0x4a, 0xda, 0xaa, 0xaa, 0xaa, 
        //  0xa0, 0x00, 0x07, 0x84, 0x55, 0x2d, 0x91, 0x2a, 0x55, 0xc1, 
        //  0x30, 0x4c, 0x13, 0x04, 0xc1, 0x30, 0x4c, 0x13, 0x04, 0xc1, 
        //  0x30, 0x4c, 0x13, 0x04, 0xc1, 0x30, 0x4c, 0x13, 0x04, 0x00, 0x00}
        val[0] = 0xc0;
        val[2] = 0xa1;
        val[3] = 0x10;
        val[6] = 0xca;
        val[13] = 0x80;
        val[14] = 0x00;
        if(trigMode == "LG")
            val[15] = 0x00;
        else
            val[15] = 0x04;
        val[17] = 0x22;
        val[18] = 0x40;
    }
    // std::cout<<"val:"<<std::hex<<val[6]<<" "<<val[10]<<std::endl;
    for(int i=0; i<41; i++)
    {
        // std::cout<<"val_send:"<<std::hex<<val[i]<<' ';
        spi_send_threshold(hw, Command, Asic_id, reg_id, val[i]);
        reg_id++;
    }

    Command = 0xC0 + int(Asic_id);
    if(!spi_send_threshold(hw, Command, 0x00, 0x00, 0x00))
        return;
 
    // infile.close();
}

//added by new format
void gcu_clear_fifo_threshold(HwInterface* hw)
{
    
    ValWord<uint32_t> count = readCounter(hw);
    
    // std::cout<<"count:"<<count.value()<<std::endl;
    
    
    ValVector<uint32_t> fifo;
    fifo = readFifo(hw, int(count));

    

    // std::cout<<"GCU FIFO CLEAR SUCCESS"<<std::endl;

}
 
//added by new format
uint32_t *gcu_read_fifo_threshold(HwInterface* hw)
{
    ValWord<uint32_t> count = readCounter(hw);
    
    // std::cout<<"count:"<<count.value()<<std::endl;
    
    ValVector<uint32_t> fifo;
    fifo = readFifo(hw, int(count));
    
    static uint32_t res[5000000];
    for(int i=0; i<fifo.size(); i++)
    {
        res[i] = (uint32_t)fifo[i];
    }
    return res;
}

//设阈值加入了判断是否设成功
//d7 85 b5 54 41 4a da aa aa aa a0 00 07 84 55 2d 91 2a 55 c1 30 4c 13 04 c1 30 4c 13 04 c1 30 4c 13 04 c1 30 4c 13 04 00 00
bool update_thresholds_catiroc(HwInterface* hw, int configCnt, uint32_t Asic_id, std::string disTrigger, std::string gcu_auto, std::string trigMode, std::string asic_flag, int *threhold_time, int *threhold_HGLG, int gcu_id, std::ofstream& logFile, Redis *m_redis)
{
    if(!logFile.is_open())
    {
        std::cout<<"Unable to open LogFile"<<std::endl;
    }

    gcu_clear_fifo_threshold(hw);
    bool Flag = false;
    uint32_t *val;
    uint32_t *val_fifo;
    uint32_t threshold_val[100];
    int cnt =0;
    while(!Flag)
    {
        cnt++;
        val = set_parameters_catirocs(hw, Asic_id, disTrigger, gcu_auto, trigMode, asic_flag, threhold_time, threhold_HGLG);  
        send_parameters_catirocs(hw, Asic_id, disTrigger, gcu_auto, trigMode, asic_flag, threhold_time, threhold_HGLG);
        threshold_val[0] = 0;
        threshold_val[1] = 0;
        int temp;
        for(int i=0; i<41; i++)
        {
            if(i%2 == 0)
                temp = val[i];
            else
            {
                temp = (temp << 8) + val[i];
                threshold_val[i/2+2] = temp;
            }
            
        }
        Flag = true;
        usleep(210);
        // sleep(0.21);
        

        // std::cout<<std::endl;

    }
    // std::cout<<gcu_id<<"Config Threshold Succcess"<<" Config Times:"<<cnt<<std::endl;
    return Flag;
    
}


// int setThreshold(HwInterface* hw, const char *SCFile, int gcu_num, int gcu_id, std::ofstream& logFile, Redis *m_redis)
int setThreshold(HwInterface* hw, int configCnt, std::string disTrigger, const std::vector<std::string>& gcu_auto, std::string trigMode, int* thresholdHG, int* threshold, const std::vector<std::string>& asic_vecs, int gcu_id, std::ofstream& logFile, Redis *m_redis)
{
    if(!logFile.is_open())
    {
        std::cout<<gcu_id<<' '<<"Unable to open logFile"<<std::endl;
    }
    int config_succ = true;
    int times = 10;
    while(times>0)
    {
        try
        {
            hw->getNode("SPI_Master_Controller").getNode("ss").write(0xFF);
            hw->dispatch();
            hw->getNode("SPI_Master_Controller").getNode("ctrl").write(0x2820);
            hw->dispatch();
            break;
        }
        catch(std::exception &e)
        {
            std::time_t now = std::time(nullptr);
    
            // 将时间戳转换为本地时间
            std::tm* local_time = std::localtime(&now);
            
            // 格式化本地时间为字符串
            char time_str[100];
            std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
            {
                // m_logmutex.lock();
                logFile<<gcu_id<<"|SPI_MASTER_CTRL_ERROR_THRE_ERROR|"<<time_str<<std::endl;
                m_redis->set("TCP_Alert", "1");
                // m_logmutex.unlock();
            }
            times--;
            if(times == 0)
                return -1;
        }
    }
    
    times = 10;

    int Asic_id = 0;
    int ans;
    // std::fstream config_file;
    // config_file.open(SCFile,std::ios::in);
    // if(!config_file.is_open())
    // {
    //     std::cout<<"Read Config File ERROR!"<<std::endl;
    // }
    // std::string line;
    // for(int i=0; i<8*gcu_num+1; i++)
    //     std::getline(config_file, line);
    for(auto asic_flag:asic_vecs)
    // for(int i=0; i<8; i++)
    {
        // std::cout<<Asic_id<<" "<<asic_flag<<std::endl;
        if(asic_flag == "SKIP")
        {
            Asic_id++;
            continue;
        }
        // std::getline(config_file, line);
        // std::vector<std::string> row;
        // Stringsplit(line, " ", row);
        // std::cout<<thresholdHG[i]<<' '<<threshold[i]<<std::endl;
        uint32_t reg_id = 0;
        int val;
        int start = 22528;
        int col = 0;
        int threhold_time[2];
        int threhold_HGLG[2];
        
        int res[16]={0};
    
        val = thresholdHG[Asic_id]; //转成整形
        // val = std::stoi(row[2]); //转成整形
        ans = val;
        if(val>=1024)
            val = val % 1024;
        val = val + start;
        
        // std::cout<<val<<std::endl;
        for(int j=0; val>0; j++)
        {
            res[j] = val%2;
            val = val/2;
        }
        
        // for(int i=0;i<16;i++)
        //     std::cout<<res[i]<<' ';
        // std::cout<<std::endl;
        // std::cout<<Command<<' '<<Asic_id<<' '<<reg_id<<std::endl;
        std::string binary1 = "";
        std::string binary2 = "";
        
        for(int j=0; j<8; j++)
        {
            binary1 += std::to_string(res[j]);
            binary2 += std::to_string(res[j+8]);
        }
        threhold_time[0] = stoi(binary1, nullptr, 2);
        threhold_time[1] = stoi(binary2, nullptr, 2);
        // std::cout<<"threhold[0]:"<<threhold_time[0]<<std::endl;
        // std::cout<<"threhold[1]:"<<threhold_time[1]<<std::endl;
    
    
        val = threshold[Asic_id];
        // val = std::stoi(row[3]);
        if(val>=1024)
            val = val % 1024;
        int temp[4] = {84,86,85,87};
        threhold_HGLG[0] = temp[val%4];
        val = val / 4;
        std::string binary = "";

        for(int j=0; val>0; j++)
        {
            res[j] = val % 2;
            val = val / 2;
        }
        for(int j=0; j<8; j++)
            binary += std::to_string(res[j]);
        
        threhold_HGLG[1] = stoi(binary, nullptr, 2);
        int cnt_thre_trig = 0;
        int cnt_thre = 0;
        while(1)
        {
            update_thresholds_catiroc(hw, configCnt, Asic_id, disTrigger, gcu_auto[Asic_id], trigMode, asic_flag, threhold_time, threhold_HGLG, gcu_id, logFile, m_redis);
            usleep(210000);
            bool flag_add = get_comparaison_status(hw, Asic_id);
            if(flag_add)
            {
                uint32_t val1 = get_triggers_counter(hw);
                if(disTrigger == "N" && val1 != 0)
                    break;
                if(disTrigger == "Y" && val1 == 0)
                    break;
                else
                {
                    
                    if(cnt_thre_trig%50 == 0)
                    {                    
                        std::cout<<"GCU:"<<gcu_id<<" CATIROC_"<<Asic_id<<" Config Error_CNT_"<<cnt_thre_trig<<std::endl;
                        // cnt_thre_trig++;
                        std::time_t now = std::time(nullptr);
            
                        // 将时间戳转换为本地时间
                        std::tm* local_time = std::localtime(&now);
                        
                        // 格式化本地时间为字符串
                        char time_str[100];
                        std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
                        {
                            // m_logmutex.lock();
                            logFile<<gcu_id<<"|CATIROC_"<<Asic_id<<"_Config_Error_CNT_"<<cnt_thre_trig<<"|"<<time_str<<std::endl;
                            // m_redis->set("TCP_Alert", "1");
                            // m_logmutex.unlock();
                        }
                    }
                    cnt_thre_trig++;
                    if(cnt_thre_trig%101 == 0)
                    {
                        config_succ = false;
                        break;
                    }
                }
            }
            
            if(cnt_thre%50==0)
            {
                std::cout<<"GCU:"<<gcu_id<<" CATIROC_"<<Asic_id<<" Flag Config Error_CNT_"<<cnt_thre<<std::endl;
                std::time_t now = std::time(nullptr);
                // 将时间戳转换为本地时间
                std::tm* local_time = std::localtime(&now);
                // 格式化本地时间为字符串
                char time_str[100];
                std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
                {
                    // m_logmutex.lock();
                    logFile<<gcu_id<<"|CATIROC_"<<Asic_id<<"_Flag_Config_Error_CNT_"<<cnt_thre<<"|"<<time_str<<std::endl;
                    // m_redis->set("TCP_Alert", "1");
                    // m_logmutex.unlock();
                }
            }
            cnt_thre++;
            if(cnt_thre%101==0)
            {
                config_succ = false;
                break;
            }    
        }
        
        std::cout<<"GCU_"<<gcu_id<<"CAT_"<<Asic_id<<"_ConfigFlag_"<<config_succ<<"_CNT_"<<cnt_thre_trig<<"_"<<cnt_thre<<std::endl;
        Asic_id++;
        //这里听了1s
        usleep(10000);
        // std::cout<<"CAT:"<<gcu_id<<
    }
    // config_file.close();

    return config_succ;
}