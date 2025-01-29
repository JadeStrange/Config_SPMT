#ifndef GCU_H
#define GCU_H

#include <vector>
#include <string>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include "uhal/uhal.hpp"
#include "Redis.h"

using namespace uhal;

void abc_init(HwInterface* hw);
void gcu_init(HwInterface* hw);
void hv_init(HwInterface* hw);

bool gcu_start_acq(HwInterface* hw);
bool gcu_stop_acq(HwInterface* hw);
bool abc_start_acq_ped(HwInterface* hw, int freqNum, int charge, int dds);
bool abc_start_acq_pm(HwInterface* hw, int charge, int dds, std::string highrateMode);
bool abc_stop_acq(HwInterface* hw);
bool gcu_clear_fifo(HwInterface* hw);
bool abc_clear_fifo(HwInterface* hw);
bool gcu_wait_for_event(HwInterface* hw, int waitTime);
void abc_master_reset(HwInterface* hw);


ValWord<uint32_t> readCounter(HwInterface* hw);


ValVector<uint32_t> readFifo(HwInterface* hw, uint32_t count);

//********SPI协议************
bool spi_send(HwInterface* hw, uint32_t byte_0, uint32_t byte_1, uint32_t byte_2, uint32_t byte_3);

// void update_thresholds_catiroc(HwInterface* hw, uint32_t Asic_id, int *threhold_time, int *threhold_HGLG);

int setThreshold(HwInterface*, int configCnt, std::string disTrigger, const std::vector<std::string>& gcu_auto, std::string m_trigMode, int *thresholdHG, int* threshold, const std::vector<std::string>& asic_vecs, int gcu_id, std::ofstream& outFile, Redis *m_redis);
// int setThreshold(HwInterface*, const char *SCFile, int gcu_num, int gcu_id, std::ofstream& outFile, Redis *m_redis);

#endif