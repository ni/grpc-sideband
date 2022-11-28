//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#ifdef __cplusplus
   extern "C" {
#endif

#if defined(_WIN32)
    #define SIDEBAND_C_CONV __cdecl
    #if defined(_BUILDING_GRPC_SIDEBAND)
        #define SIDEBAND_IMPORT_EXPORT __declspec(dllexport)
    #else
        #define SIDEBAND_IMPORT_EXPORT __declspec(dllimport)
    #endif
#else
    #define SIDEBAND_C_CONV
    #if defined(_BUILDING_GRPC_SIDEBAND)
        #define SIDEBAND_IMPORT_EXPORT __attribute__ ((section (".export")))
    #else
        #define SIDEBAND_IMPORT_EXPORT
    #endif
#endif
#define  _SIDEBAND_FUNC SIDEBAND_IMPORT_EXPORT SIDEBAND_C_CONV

//---------------------------------------------------------------------
//---------------------------------------------------------------------
enum class SidebandStrategy
{
  UNKNOWN = 0,
  GRPC = 1,
  SHARED_MEMORY = 2,
  DOUBLE_BUFFERED_SHARED_MEMORY = 3,
  SOCKETS = 4,
  SOCKETS_LOW_LATENCY = 5,
  HYPERVISOR_SOCKETS = 6,
  RDMA = 7,
  RDMA_LOW_LATENCY = 8
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC InitOwnerSidebandData(::SidebandStrategy strategy, int64_t bufferSize, char* out_sideband_id);
int32_t _SIDEBAND_FUNC GetOwnerSidebandDataToken(const char* usageId, int32_t* out_tokenId);
int32_t _SIDEBAND_FUNC InitClientSidebandData(const char* sidebandServiceUrl, ::SidebandStrategy strategy, const char* usageId, int bufferSize, int64_t* out_tokenId);
int32_t _SIDEBAND_FUNC WriteSidebandData(int64_t dataToken, uint8_t* bytes, int64_t bytecount);
int32_t _SIDEBAND_FUNC ReadSidebandData(int64_t dataToken, uint8_t* bytes, int64_t bufferSize, int64_t* numBytesRead);
int32_t _SIDEBAND_FUNC CloseSidebandData(int64_t dataToken);

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC RunSidebandSocketsAccept(const char* address, int port);
int32_t _SIDEBAND_FUNC AcceptSidebandRdmaSendRequests();
int32_t _SIDEBAND_FUNC AcceptSidebandRdmaReceiveRequests();

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC QueueSidebandConnection(::SidebandStrategy strategy, const char* id, bool waitForReader, bool waitForWriter, int64_t bufferSize);

#ifdef __cplusplus
   }
#endif
