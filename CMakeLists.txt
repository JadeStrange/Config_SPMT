cmake_minimum_required(VERSION 3.10)
project(Config_SPMT)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR}/Config_SPMT)
aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

include_directories($ENV{IPBUS_PATH}/include)
link_directories($ENV{IPBUS_PATH}/lib)
link_libraries( cactus_uhal_log cactus_uhal_grammars cactus_uhal_uhal )

include_directories($ENV{CMAKE_PREFIX_PATH}/include)
link_directories($ENV{CMAKE_PREFIX_PATH}/lib)
link_directories($ENV{CMAKE_PREFIX_PATH}/lib64)
link_libraries(hiredis)

add_executable(Config_SPMT ${SRC})
target_link_libraries (${PROJECT_NAME} pthread cactus_uhal_log cactus_uhal_grammars cactus_uhal_uhal boost_system hiredis) 
