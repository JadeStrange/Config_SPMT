#ifndef IPBUS_GCU_CONFIG_H
#define IPBUS_GCU_CONFIG_H

#include "uhal/uhal.hpp"
#include <string>
#include <thread>
#include <fstream>
#include "Redis.h"
#include "spdlog/spdlog.h"

#define CAT_NUM 8
namespace gcu
{
    class IPBUS_GCU_config
    {
        private:
            uhal::HwInterface* hw;
            bool config_succ;
            bool done_flag;
            // Redis*  m_redis = nullptr;

            int m_gcuId;
            std::string m_gcuIp;
            int m_threshold[8];
            int m_thresholdHG[8];
            std::vector<std::string> m_catConfig;
            int m_freqValue;
            std::string m_trgMode;
            std::string m_disTrigger;
            std::vector<std::string> m_gcuAuto;
            std::string m_enableHighRate;
            int m_mode;
            const int TAP_NUM = 125;
            std::vector<int> bert;
            int m_BEC_port;

            std::shared_ptr<std::thread> m_thread;

            //日志文件
            std::ofstream m_logFile;
            std::mutex m_logmutex;
            Redis* m_redis;

            static uhal::ConnectionManager manager;

        protected:
            void connect_gcu();
            
            void elec_config();
            void synchronize();
            void start_trigger();
            void stop_trigger();
            void reset();
            void close_gcu();
            void eye_scan();

        public:
            IPBUS_GCU_config(const std::map<std::string, std::string> &cfg_val, std::string logDirname, int mode);
            virtual ~IPBUS_GCU_config();

            void start(std::string cmd);
            void stop(std::string cmd);
            int get_flag();

    };


} // namespace gcu

#endif