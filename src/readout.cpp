#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <time.h>
#include "uhal/uhal.hpp"
#include "boost/filesystem.hpp"
#include "readout.h"
#include <math.h>
#include <arpa/inet.h>

//using namespace uhal;
//using namespace std;

std::vector<uint16_t> L1id;
int trailer[]={0x55aa,0x0123,0x4567,0x89ab,0xcdef,0xff00};

bool Read(HwInterface* hw,std::string nodename,uint32_t N,uint32_t* p,char* filename)
{
    uint32_t TIMES=0;
    //while(tag==1)
    //{
    ValVector< uint32_t > mem;
    int size = N;
    int occupied_current = 2000;
    int count,occupied;
    while(1)
    {
        sleep(1);
        try
        {
            try
            {
                hw->dispatch();
            }
            catch(std::exception &e)
            {
                //std::cout<<"BAD"<<std::endl;
                exit(1);
            }
            //std::cout<<"read"<<std::endl;
            hw->getNode("ipbus_fifo.ctrl").write(0x1);
            // hw->dispatch();

            //std::cout<<"over"<<std::endl;
            count = hw->getNode("ipbus_fifo.status.count").read();
            // hw->dispatch();
            //std::cout<<"over2"<<std::endl;
            occupied = hw->getNode("ipbus_fifo.occupied").read();
            hw->dispatch();
            //std::cout<<"over3"<<std::endl;
            if (occupied == 0)
            {
            }
            else
                occupied_current = occupied;
            if (occupied_current < 2000)
            {
                size = size - 10;
                sleep(0.01);
            }
            else
            {
                size = size + 10;
            }
            //std::cout<<"start"<<std::endl;
            mem=hw->getNode(nodename).readBlock(size);
            ValWord<uint32_t> count = hw->getNode("ipbus_fifo.status.count").read();
            hw->getNode("ipbus_fifo.ctrl").write(0);
            hw->dispatch();
            int control = hw->getNode("ipbus_fifo.ctrl").read();
            hw->dispatch();
            //std::cout<<"stop"<<std::endl;
            ////std::cout<<"read out data"<<std::endl;
            if(count==0)
            {
                for(uint32_t i=0;i<N;i++)
                {
                    //std::cout<<std::hex<<mem[i]<<" ";
                }
                //std::cout<<std::endl;
            }

            /*
                 for(uint32_t i=0;i<N;i++)
                 {
             *(p+i)=mem[i];
             }
             */
            /*
               //std::cout<<"pHHHHHHHH"<<std::endl;
               for(uint32_t i=0;i<N;i++)
               {
               //std::cout<<std::hex<<p[i]<<" ";
               }
               //std::cout<<std::endl;
               */
        }
        catch(std::exception &e)
        {
            WriteLog("READ ERROR"+std::string(e.what()),NULL);
            ////std::cout<<"read err msg:"<<e.what()<<std::endl;
            continue;
        }
        /*
           for(uint32_t i=0;i<N;i+=4)
           {
           Exchange(p+i);
           }
           */
        /*
           //std::cout<<"!!!!!!"<<std::endl;
           for(uint32_t i=0;i<N;i++)
           {
           //std::cout<<std::hex<<p[i]<<" ";
           }
           //std::cout<<std::endl;
           */
        /*
           if(TIMES==0)
           {
           L1id.push_back((uint16_t)(p[1]&0x0000ffff));
           }
           */
        //Check(p,N,TIMES);
        //Save(p,filename,N);
        TIMES++;
    }
    return 0;
}

//check the trailer
bool seq_in_seq(uint16_t* p, int seq_length,int& index_count)  //p:the raw data point seq_length:the raw data length index_count:represent the 55aa position(0,1,2..) when return true
{
    int index=0;
    while(index<seq_length && seq_length>(sizeof(trailer)/sizeof(trailer[0])))
    {
        if(*(p+index)==trailer[0])
        {
            index_count=index_count+index+1;
            for(int i=0;i<(sizeof(trailer)/sizeof(trailer[0]));i++)
            {
                if(*(p+index+i)!=trailer[i])
                {
                    p=p+index+1;
                    seq_length=seq_length-index-1;
                    if(!seq_in_seq(p,seq_length,index_count))
                        return false;
                    else
                        return true;
                }
            }
            return true;
        }
        index++;

    }
    return 1;
}

//trigger not used
void trigger(HwInterface* hw,uint32_t ctrl)
{
    WriteRead_Reg(hw,"cs_write.command","cs_write.command",ctrl);
    WriteRead_Reg(hw,"cs_write.ctrl.chb_req2","cs_write.ctrl.chb_req2",0x1);
    usleep(1000);
    while(1)
    {
        ValWord<uint32_t> m=hw->getNode("cs_read.status.chb_grant2").read();
        hw->dispatch();
        if(m==1)
            break;
    }
    //std::cout<<"!!!!!!!!!!!!!!"<<std::endl;
    WriteRead_Reg(hw,"cs_write.ctrl.chb_req2","cs_write.ctrl.chb_req2",0x0);
    WriteRead_Reg(hw,"cs_write.ctrl.dbg_strobe","cs_write.ctrl.dbg_strobe",0x1);
    WriteRead_Reg(hw,"cs_write.ctrl.dbg_strobe","cs_write.ctrl.dbg_strobe",0x0);
}


void Save_bin(uint32_t* p,const char*filename,uint32_t N)
{
    std::ofstream save(filename,std::ios::binary);
    if(!save)
    {
        //std::cout<<"error open"<<std::endl;
    }
    save.write((char*)p,4*N);
    save.close();
}


bool save_bin(uint16_t* p,std::ofstream* save,uint32_t N)
{
    if(!(*save))
    {
        //std::cout<<"error open"<<std::endl;
    }
    save->write((const char *)p,2*N);
    return true;
}


bool save_text(uint16_t * p,std::ofstream* save, uint32_t N)
{
    try
    {
        //save_bin(p,filename.c_str(),size);
        //                        //save_tex(p,filename.c_str(),size);
        //std::ofstream save(filename.c_str(),std::ios::app);
        if(!(*save))
        {
            //std::cout<<"error open"<<std::endl;
        }
        for(uint32_t i=0;i<N;i++)
        {
            (*save)<<"0x"<<std::hex<<*(p+i)<<"\n";
        }
        //(*save)<<"time"<<"\n";
        *save<<std::flush;
        return true;
    }
    catch(std::exception &e)
    {
        //WriteLog("READ ERROR"+std::string(e.what()),NULL);
        //std::cout<<"write err, msg:"<<e.what()<<std::endl;
        return false;
    }
}


bool WriteRead_Reg(HwInterface* hw,std::string nodename, std::string nodename_rd,uint32_t value)
{
    bool eaq_flag=false;
    int time=10;
    while(time > 0)
    {
        try
        {
            hw->getNode(nodename).write(value);
            hw->dispatch();
            usleep(1000);
            ValWord<uint32_t> Reg=hw->getNode(nodename_rd).read();
            hw->dispatch();
            ////std::cout << std::hex << Reg.value() << std::endl;
            //u_int32_t val = Reg.value();
            u_int32_t val = Reg.value() << 12 >> 12;
            if(val == value)
            {
                return true;
            }
            else
            {
                eaq_flag=true;
                throw;
            }
        }
        catch(std::exception &e)
        {
            time--;
            if(eaq_flag)
            {
                //std::cout<<"Write != read ,"<<nodename<<"error!";
                eaq_flag=false;
            }
            else 
            {
                if (time == 9)
                    //std::cout<<"Write read register "<<nodename<<"error 1 time!";
                usleep(10000);
            }
            continue;
        }
    }
    //std::cout<<"Write read register "<<nodename<<"fail!";
    return false;
}

bool WriteRead_Reg_s(HwInterface* hw,std::string nodename,uint32_t value)
{
    bool eaq_flag=false;
    int time=10;
    while(time > 0)
    {
        try
        {
            hw->getNode(nodename).write(value);
            hw->dispatch();
            usleep(1000);
            ValWord<uint32_t> Reg=hw->getNode(nodename + "_rd").read();
            hw->dispatch();
            ////std::cout << std::hex << Reg.value() << std::endl;
            u_int32_t val = Reg.value() << 30 >> 30;
            
            if(val == value)
            {
                return true;
            }
            else
            {
                eaq_flag=true;
                throw;
            }
        }
        catch(std::exception &e)
        {
            time--;
            if(eaq_flag)
            {
                //std::cout<<"Write != read ,"<<nodename<<"error!";
                eaq_flag=false;
            }
            else 
            {
                if (time == 9)
                    //std::cout<<"Write read register "<<nodename<<"error 1 time!";
                usleep(10000);
            }
            continue;
        }
    }
    //std::cout<<"Write read register "<<nodename<<"fail!";
    return false;
}

bool WriteReg(HwInterface* hw,std::string nodename,uint32_t value) //32bits register
{
    int time = 10;
    while (time > 0)
    {
        try
        {
            hw->getNode(nodename).write(value);
            hw->dispatch();
        }
        catch(std::exception& e)
        {
            time --;
            if (time == 9)
                //std::cout<<"Write register "<<nodename<<"error 1 time!";
            usleep(10000);
            continue;
        }
        return true;
    }
    //std::cout<<"Write register "<<nodename<<"fail!";
    return false;
}


ValWord<uint32_t> ReadReg(HwInterface* hw,std::string nodename)
{
    int time = 10;
    while(time > 0)
    {
        ValWord<uint32_t> ret;
        try
        {
            ret=hw->getNode(nodename).read();
            hw->dispatch();
            // u_int32_t val = ret.value() << 12 >> 12;
            // //std::cout << val << std::endl;
        }
        catch(std::exception& e)
        {
            time --;
            if (time == 9)
                //std::cout<<"Read register "<<nodename<<"error 1 time!";
            usleep(10000);
            continue;
        }
        return ret;
    }
    //std::cout<<"Read register "<<nodename<<"fail!";
    return 0;
}


bool WriteBlock(HwInterface* hw,string nodename,vector<uint32_t> value)
{
    try
    {
        hw->getNode(nodename).writeBlock(value);
        hw->dispatch();
        return 1;
    }
    catch(std::exception& e)
    {
        //std::cout<<"Write block "<<nodename<<",size= "<<value.size()<<",error!";
        return 0;
    }   
}


ValVector<uint32_t> ReadBlock(HwInterface* hw,string nodename,uint32_t num)
{
    try
    {
        ValVector<uint32_t> mem=hw->getNode(nodename).readBlock(num);
        hw->dispatch();
        return mem;
        /*
           for(size_t i=0;i!=num;i++)
           {
         *(p+i)=(uint32_t)mem[i];
         }
         */
    }
    catch(std::exception& e)
    {
        //std::cout<<"Read block "<<nodename<<",size= "<<num<<",error!";
    }
    return 0;
}


bool try_dispatch(HwInterface* hw)
{
    try
    {
        hw->dispatch();
    }
    catch(std::exception& e)
    {
        //std::cout<<"dispatch error"<<std::endl;
        //logger
        return false;
    }
    return true;
}


void Exchange(uint32_t *p)   //4words(128bits):1 2 3 4-->4 3 2 1
{
    uint32_t temp;
    temp=p[0];
    p[0]=p[3];
    p[3]=temp;
    temp=p[1];
    p[1]=p[2];
    p[2]=temp;
}




/*uint16_t exchange(uint16_t buf)    //abcd --> cdab
{
    uint8_t buffer[2];

}*/


bool Check(uint32_t* p,uint32_t N,uint32_t TIMES)
{

    if((p[0]>>16)!=0x805a)    //header
    {
        WriteLog("error head",NULL);
    }
    if((p[N-1]&0x0000ffff)!=0x8069)    //trailer
    {
        WriteLog("error tail",NULL);
    }
    if(TIMES!=0)
    {
        Check_l1id(p,N);
    }
    Check_data(p,N);
    return true;
}


bool Check_data(uint32_t* p,uint32_t N)
{

    uint16_t temp[2*(N-8)];

    for (uint32_t i=0;i<N-8;i++)
    {
        temp[2*i+1]=(uint16_t)((p[i+4])&(0x0000ffff));
        temp[2*i]=(uint16_t)(p[i+4]>>16);
    }

    for(uint32_t k=0;k<2*(N-8);k++)
    {
        if((k+1)<(2*(N-8)))
        {
            if((temp[k]+1)!=temp[k+1])
            {
                WriteLog("point not right",NULL);
                return false;
            }
        }
    }
    return true;
}


bool Check_l1id(uint32_t* p,uint32_t N)
{
    uint16_t l1id=(p[1]&0x0000ffff);
    if((l1id-1)!=L1id.back())
    {
        //std::stringstream ss;
        //std::string s;
        //ss<<l1id;
        //ss>>s;

        WriteLog("error l1id:now:",&l1id);
        WriteLog(" error l1id:last:",&(L1id.back()));
        WriteLog("\n",NULL);
        //std::cout<<"error l1id"<<"last:"<<L1id.back()<<"now:"<<l1id<<std::endl;
        return false;
    }
    L1id.back()=l1id;
    return true;
}


bool Save(uint32_t* p,char* filename,uint32_t N)
{
    //text file

    std::ofstream save(filename);
    if(!save)
    {
        //std::cout<<"error open"<<std::endl;
    }
    for(uint32_t i=0;i<N;i++)
    {
        save<<std::hex<<*(p+i)<<" ";
    }
    //save<<"\n";
    save<<std::flush;
    save.close();

    //binary file
    /*
       std::ofstream save(filename,std::ios::binary|std::ios::app);
       if(!save)
       {
       //std::cout<<"error open"<<std::endl;
       }
       save.write((const char *)p,4*N);
       */
    return 1;
}


void WriteLog(std::string szlog,uint16_t*  num)
{
    // time_t timep;
    // time(&timep);
    FILE *fp;
    fp=fopen("./log.txt","a");
    // fprintf(fp,"MyLogInfo:%s:",asctime(gmtime(&timep)));
    const char *SZlog=szlog.c_str();
    fprintf(fp,SZlog);
    if(num!=NULL)
    {
        fprintf(fp,"%x",*num);
    }
    fclose(fp);
}


void config_read_temp(HwInterface& hw)
{

    ValWord<uint32_t> status = hw.getNode("cs_read.status").read();
    ValWord<uint32_t> locked_tx = hw.getNode("cs_read.status.locked_tx").read();
    ValWord<uint32_t> locked_rx = hw.getNode("cs_read.status.locked_rx").read();
    ValWord<uint32_t> ttc_tx_ready = hw.getNode("cs_read.status.ttc_tx_ready").read();
    ValWord<uint32_t> ttc_rx_ready = hw.getNode("cs_read.status.ttc_rx_ready").read();
    ValWord<uint32_t> no_errors = hw.getNode("cs_read.status.no_errors").read();
    ValWord<uint32_t> aligned = hw.getNode("cs_read.status.aligned").read();
    ValWord<uint32_t> ddr3_init_done = hw.getNode("cs_read.status.ddr3_init_done").read();
    ValWord<uint32_t> tdc_intb = hw.getNode("cs_read.status.tdc_intb").read();
    ValWord<uint32_t> ddr3_tg_compare_error = hw.getNode("cs_read.status.ddr3_tg_compare_error").read();
    ValWord<uint32_t> ddr3_power_good = hw.getNode("cs_read.status.ddr3_power_good").read();
    ValWord<uint32_t> fmc_init_done = hw.getNode("cs_read.status.fmc_init_done").read();

    hw.dispatch();

    //std::cout<<"locked_tx: "<<locked_tx<<std::endl;
    //std::cout<<"locked_rx: "<<locked_rx<<std::endl;
    //std::cout<<"ttc_tx_ready: "<<ttc_tx_ready<<std::endl;
    //std::cout<<"ttc_rx_ready: "<<ttc_rx_ready<<std::endl;
    //std::cout<<"no_errors: "<<no_errors<<std::endl;
    //std::cout<<"aligned: "<<aligned<<std::endl;
    //std::cout<<"ddr3_init_done: "<<ddr3_init_done<<std::endl;
    //std::cout<<"tdc_intb: "<<tdc_intb<<std::endl;
    //std::cout<<"ddr3_tg_compare_error: "<<ddr3_tg_compare_error<<std::endl;
    //std::cout<<"ddr3_power_good: "<<ddr3_power_good<<std::endl;
    //std::cout<<"fmc_init_done: "<<fmc_init_done<<std::endl;

    //create temperature horizontal bar histogram
    // I2C core initialization
    hw.getNode("i2c_master.prerl").write(0x80);  //PRESCALER byte0
    hw.getNode("i2c_master.prerh").write(0x00);   // PRESCALER byte1
    hw.getNode("i2c_master.ctrl").write(0x80);    // I2C core enabled
    hw.dispatch();
    temp(hw,0x30,0x90);
    temp(hw,0xfe,0x10);
    temp(hw,0x31,0x90);
    hw.getNode("i2c_master.command").write(0x28);  //issue a read command + STOP bit
    hw.dispatch();
    int i = 0;
    while (i < 1)    // wait transfer is in progress
    {
        ValWord<uint32_t> tip = hw.getNode("i2c_master.status").read();
        hw.dispatch();
        if (tip == 0x00000000)
            i = 1;
    }
    ValWord<uint32_t> manufacturer_ID1 = hw.getNode("i2c_master.rx").read();
    hw.dispatch();
    //std::cout<<"MAX1617a chip 1 manufacturer ID: "<<std::hex<<manufacturer_ID1<<std::endl;
    temp(hw,0x30,0x90);
    temp(hw,0xff,0x10);
    temp(hw,0x31,0x90);
    hw.getNode("i2c_master.command").write(0x28);// issue a read command + STOP bit
    hw.dispatch();
    i = 0;
    while (i < 1)    // wait transfer is in progress
    {
        ValWord<uint32_t> tip = hw.getNode("i2c_master.status").read();
        hw.dispatch();
        if (tip == 0x00000000)
            i = 1;
    }

    ValWord<uint32_t> device_ID1 = hw.getNode("i2c_master.rx").read();
    hw.dispatch();
    //std::cout<<"MAX1617a chip 1 device ID: "<<std::hex<<device_ID1<<std::endl;

    temp(hw,0x30,0x90);
    temp(hw,0x00,0x10);
    temp(hw,0x31,0x90);
    hw.getNode("i2c_master.command").write(0x28);// issue a read command + STOP bit
    hw.dispatch();
    i = 0;
    while (i < 1)    // wait transfer is in progress
    {
        ValWord<uint32_t> tip = hw.getNode("i2c_master.status").read();
        hw.dispatch();
        if (tip == 0x00000000)
            i = 1;
    }

    ValWord<uint32_t> local_temp_chip1 = hw.getNode("i2c_master.rx").read();
    hw.dispatch();
    //std::cout<<"MAX1617a chip 1 local temperature: "<<local_temp_chip1<<std::endl;
    temp(hw,0x30,0x90);
    temp(hw,0x01,0x10);
    temp(hw,0x31,0x90);
    hw.getNode("i2c_master.command").write(0x28);// issue a read command + STOP bit
    hw.dispatch();
    i = 0;
    while (i < 1)    // wait transfer is in progress
    {
        ValWord<uint32_t> tip = hw.getNode("i2c_master.status").read();
        hw.dispatch();
        if (tip == 0x00000000)
            i = 1;
    }

    ValWord<uint32_t> external_temp_chip1 = hw.getNode("i2c_master.rx").read();
    hw.dispatch();
    //std::cout<<"MAX1617a chip 1 external temperature: "<<external_temp_chip1<<std::endl;
    //chip 2
    temp(hw,0x98,0x90);
    temp(hw,0xfe,0x10);
    temp(hw,0x99,0x90);
    hw.getNode("i2c_master.command").write(0x28);  //issue a read command + STOP bit
    hw.dispatch();
    i = 0;
    while (i < 1)    // wait transfer is in progress
    {
        ValWord<uint32_t> tip = hw.getNode("i2c_master.status").read();
        hw.dispatch();
        if (tip == 0x00000000)
            i = 1;
    }
    ValWord<uint32_t> manufacturer_ID2 = hw.getNode("i2c_master.rx").read();
    hw.dispatch();
    //std::cout<<"MAX1617a chip 2 manufacturer ID: "<<std::hex<<manufacturer_ID2<<std::endl;
    temp(hw,0x98,0x90);
    temp(hw,0xff,0x10);
    temp(hw,0x99,0x90);
    hw.getNode("i2c_master.command").write(0x28);// issue a read command + STOP bit
    hw.dispatch();
    i = 0;
    while (i < 1)    // wait transfer is in progress
    {
        ValWord<uint32_t> tip = hw.getNode("i2c_master.status").read();
        hw.dispatch();
        if (tip == 0x00000000)
            i = 1;
    }

    ValWord<uint32_t> device_ID2 = hw.getNode("i2c_master.rx").read();
    hw.dispatch();
    //std::cout<<"MAX1617a chip 2 device ID: "<<std::hex<<device_ID2<<std::endl;



    temp(hw,0x98,0x90);
    temp(hw,0x00,0x10);
    temp(hw,0x99,0x90);
    hw.getNode("i2c_master.command").write(0x28);// issue a read command + STOP bit
    hw.dispatch();
    i = 0;
    while (i < 1)    // wait transfer is in progress
    {
        ValWord<uint32_t> tip = hw.getNode("i2c_master.status").read();
        hw.dispatch();
        if (tip == 0x00000000)
            i = 1;
    }

    ValWord<uint32_t> local_temp_chip2 = hw.getNode("i2c_master.rx").read();
    hw.dispatch();
    //std::cout<<"MAX1617a chip 2 local temperature: "<<local_temp_chip2<<std::endl;
    temp(hw,0x98,0x90);
    temp(hw,0x01,0x10);
    temp(hw,0x99,0x90);
    hw.getNode("i2c_master.command").write(0x28);// issue a read command + STOP bit
    hw.dispatch();
    i = 0;
    while (i < 1)    // wait transfer is in progress
    {
        ValWord<uint32_t> tip = hw.getNode("i2c_master.status").read();
        hw.dispatch();
        if (tip == 0x00000000)
            i = 1;
    }

    ValWord<uint32_t> external_temp_chip2 = hw.getNode("i2c_master.rx").read();
    hw.dispatch();
    //std::cout<<"MAX1617a chip 2 external temperature: "<<external_temp_chip2<<std::endl;
}

void temp(HwInterface &hw,uint32_t val1,uint32_t val2)
{
    //MAX1617A chip 1 - read manufacturer ID:
    hw.getNode("i2c_master.tx").write(val1);        //slave address + write bit
    hw.getNode("i2c_master.command").write(val2); // START + enable WRITE to slave
    hw.dispatch();
    int i = 0;
    while (i < 1)    // wait transfer is in progress
    {
        ValWord<uint32_t> tip = hw.getNode("i2c_master.status").read();
        hw.dispatch();
        if (tip == 0x00000000);
        i = 1;
    }
}





