#----------------------------------------------------------------------
#----------------------------------------------------------------------
cmake_minimum_required(VERSION 3.5.1)

project(ni-grpc-sideband C CXX)

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  set(RDMA_LIB "libnirdma.so")
  SET(LVRT_LIB "liblvrt.so")
else()
  cmake_policy(SET CMP0091 NEW)
  add_definitions(-D_WIN32_WINNT=0x600 -bigobj)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267 /wd4244 /wd4018")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  set(RDMA_LIB "rdma.lib")
  SET(LVRT_LIB "")
endif()

add_definitions(-D_BUILDING_GRPC_SIDEBAND)

find_package(Threads REQUIRED)
link_directories(.)

#----------------------------------------------------------------------
# Include generated *.pb.h files
#----------------------------------------------------------------------
include_directories("${CMAKE_CURRENT_BINARY_DIR}" "./src")

#----------------------------------------------------------------------
# perftest gRPC Server
#----------------------------------------------------------------------
add_library(ni_grpc_sideband SHARED
  src/sideband_data.cc
  src/sideband_sockets.cc
  src/sideband_shared_memory.cc
  src/sideband_rdma.cc
  )
target_link_libraries(ni_grpc_sideband
  ${_REFLECTION}
  import/easyrdma
  )
