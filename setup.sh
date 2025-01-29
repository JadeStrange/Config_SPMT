source /opt/daq/daqv2.0/external/installed/root6-24/bin/thisroot.sh
source /opt/rh/devtoolset-8/enable bash


export DAQ_HOME=/opt/daq/daqv2.0
export DAQ_EXTERNAL=$DAQ_HOME/external
export CMAKE_PREFIX_PATH=${DAQ_EXTERNAL}/installed
export IPBUS_PATH=/opt/cactus

export LD_LIBRARY_PATH=${CMAKE_PREFIX_PATH}/lib:${CMAKE_PREFIX_PATH}/lib64:${IPBUS_PATH}/lib:${LD_LIBRARY_PATH}
export LIBRARY_PATH=${CMAKE_PREFIX_PATH}/lib:${LIBRARY_PATH}


export REDIS_IP=10.3.192.121
export REDIS_PORT=8088

export DATA_PATH=/home/run/SPMT_zhangsh/tcp_spmt/single_SPMT_v3/data/
# export DATA_PATH=/home/run/JUNO_data/SPMT_data/hvon_0116/
export CONFIG_PATH=/home/run/SPMT_zhangsh/tcp_spmt/Config_SPMT/cfg/cfg_PM/DAQ_Config.txt
export RUNNUMBER_PATH=/home/run/SPMT_zhangsh/tcp_spmt/Config_SPMT/cfg/DAQ_RunNumber.txt
export HTML_PATH=/home/run/SPMT_zhangsh/tcp_spmt/Config_SPMT/cfg/htmlConfig.txt