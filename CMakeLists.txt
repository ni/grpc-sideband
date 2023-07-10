#----------------------------------------------------------------------
#----------------------------------------------------------------------
cmake_minimum_required(VERSION 3.5.1)

project(ni-grpc-sideband C CXX)

option(INCLUDE_SIDEBAND_RDMA "Include support for RDMA sideband transfers" ON)

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  cmake_policy(SET CMP0091 NEW)
  add_definitions(-D_WIN32_WINNT=0x600 -bigobj)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267 /wd4244 /wd4018")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_definitions(-D_BUILDING_GRPC_SIDEBAND)

if (INCLUDE_SIDEBAND_RDMA)
  add_definitions(-DENABLE_RDMA_SIDEBAND)
endif()

if (INCLUDE_SIDEBAND_RDMA)
  add_subdirectory(third_party/easyrdma ${CMAKE_CURRENT_BINARY_DIR}/easyrdma)
endif()

find_package(Threads REQUIRED)
link_directories("." "import")

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
if (INCLUDE_SIDEBAND_RDMA)
  target_link_libraries(ni_grpc_sideband
    ${_REFLECTION}
    easyrdma
  )
else()
  target_link_libraries(ni_grpc_sideband
    ${_REFLECTION}
  )
endif()
