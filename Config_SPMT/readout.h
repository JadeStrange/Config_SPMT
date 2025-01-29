#ifndef READOUT_H
#define READOUT_H

#include <string>
#include <vector>
#include "uhal/uhal.hpp"

using namespace uhal;
using namespace std;

extern int trailer[];
bool WriteReg(HwInterface* hw, string nodename, uint32_t value);
ValWord<uint32_t> ReadReg(HwInterface* hw, string nodename);
bool WriteBlock(HwInterface* hw, string nodename, vector<uint32_t> value);
ValVector<uint32_t> ReadBlock(HwInterface* hw, string nodename, uint32_t num);
bool try_dispatch(HwInterface* hw);
bool seq_in_seq(uint16_t* p, int seq_length, int& index_count);

extern std::vector<uint16_t> L1id;
bool Read(HwInterface* hw, std::string nodename, uint32_t N, uint32_t* p, char* filename);
bool WriteRead_Reg(HwInterface* hw, std::string nodename, std::string nodename_rd, uint32_t num);
bool WriteRead_Reg_s(HwInterface* hw, std::string nodename, uint32_t num);
void Exchange(uint32_t* p);
bool Check(uint32_t* p, uint32_t N, uint32_t TIMES);
bool Save(uint32_t* p, char* filename, uint32_t N);
bool save_text(uint16_t* p, std::ofstream* save, uint32_t N);
bool save_bin(uint16_t* p, std::ofstream* save, uint32_t N);

bool Check_data(uint32_t* p, uint32_t N);
bool Check_l1id(uint32_t* p, uint32_t N);
void WriteLog(std::string log, uint16_t* num);
void config_read_temp(HwInterface& hw);
void temp(HwInterface& hw, uint32_t val1, uint32_t val2);

#endif // READOUT_H
