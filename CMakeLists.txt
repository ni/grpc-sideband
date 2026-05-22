#----------------------------------------------------------------------
#----------------------------------------------------------------------
cmake_minimum_required(VERSION 3.12.0)

project(ni-grpc-sideband
  VERSION 0.1.0
  LANGUAGES C CXX
)

include(GNUInstallDirs)

option(INCLUDE_SIDEBAND_RDMA "Include support for RDMA sideband transfers" ON)
option(SIDEBAND_STATIC "Build static library, disabled by default" OFF)
option(USE_SUBMODULE_DEPENDENCIES "Use bundled third-party libraries from the /third_party directory instead of distribution libraries" ON)

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
  if (USE_SUBMODULE_DEPENDENCIES)
    add_subdirectory(third_party/easyrdma ${CMAKE_CURRENT_BINARY_DIR}/easyrdma)
  else()
    find_library(EASYRDMA_LIBRARY REQUIRED NAMES easyrdma)
    find_path(EASYRDMA_INCLUDE_DIR REQUIRED NAMES easyrdma.h)
    add_library(easyrdma UNKNOWN IMPORTED)
    set_target_properties(easyrdma PROPERTIES
      IMPORTED_LOCATION "${EASYRDMA_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${EASYRDMA_INCLUDE_DIR}")
  endif()
endif()

find_package(Threads REQUIRED)

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

set_target_properties(ni_grpc_sideband PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR})

target_include_directories(ni_grpc_sideband
  PUBLIC
    src
    moniker_src)

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

install(TARGETS ni_grpc_sideband
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES
  src/sideband_data.h
  src/sideband_grpc.h
  src/sideband_internal.h
  src/sideband_semaphore.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

install(FILES
  moniker_src/moniker_service.h
  moniker_src/data_moniker.proto
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
