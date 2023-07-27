#----------------------------------------------------------------------
#----------------------------------------------------------------------
cmake_minimum_required(VERSION 3.5.1)

project(ni-grpc-sideband C CXX)

option(INCLUDE_SIDEBAND_RDMA "Include support for RDMA sideband transfers" ON)
option(SIDEBAND_INSTALL "Create a make install target" ON)
option(SIDEBAND_STATIC "Build static library, disabled by default" OFF)

set(SIDEBAND_INSTALL_internal SIDEBAND_INSTALL)
if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # Disable SIDEBAND_INSTALL by default if building as a submodule
  set(SIDEBAND_INSTALL_internal OFF)
endif()

set(SIDEBAND_INSTALL_INCLUDEDIR "include" CACHE STRING "Installation directory for headers")
set(SIDEBAND_INSTALL_LIBDIR "lib" CACHE STRING "Installation directory for libraries")
set(SIDEBAND_INSTALL_PROTODIR "proto" CACHE STRING "Installation directory for proto files")

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
if(SIDEBAND_STATIC)
  set(LIB_TYPE STATIC)
else()
  set(LIB_TYPE SHARED)
endif()

add_library(ni_grpc_sideband ${LIB_TYPE}
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

if(SIDEBAND_INSTALL_internal)
  set(INSTALL_HEADERS
    src/sideband_grpc.h
    src/sideband_data.h
    src/sideband_internal.h
    src/semaphore.h
  )

  foreach(_hdr ${INSTALL_HEADERS})
    string(REPLACE "src/" "" _path ${_hdr})
    get_filename_component(_path ${_path} PATH)
    install(FILES ${_hdr}
      DESTINATION "${SIDEBAND_INSTALL_INCLUDEDIR}/${_path}"
    )
  endforeach()

  set(PROTO_FILES
    moniker_src/data_moniker.proto
  )

  foreach(_f ${PROTO_FILES})
    string(REPLACE "moniker_src/" "" _path ${_f})
    get_filename_component(_path ${_path} PATH)
    install(FILES ${_f}
      DESTINATION "${SIDEBAND_INSTALL_PROTODIR}/${_path}"
    )
  endforeach()

  install(TARGETS ni_grpc_sideband
    DESTINATION ${SIDEBAND_INSTALL_LIBDIR}
  )
endif()
