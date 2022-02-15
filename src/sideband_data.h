//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <string>

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
std::string InitOwnerSidebandData(::SidebandStrategy strategy, int64_t bufferSize);
int64_t GetOwnerSidebandDataToken(const std::string& usageId);
int64_t InitClientSidebandData(const std::string& sidebandServiceUrl, ::SidebandStrategy strategy, const std::string& usageId, int bufferSize);
void WriteSidebandData(int64_t dataToken, uint8_t* bytes, int64_t bytecount);
void ReadSidebandData(int64_t dataToken, uint8_t* bytes, int64_t bufferSize, int64_t* numBytesRead);
void CloseSidebandData(int64_t dataToken);

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int RunSidebandSocketsAccept(int port);
int AcceptSidebandRdmaSendRequests();
int AcceptSidebandRdmaReceiveRequests();

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void QueueSidebandConnection(::SidebandStrategy strategy, const std::string& id, bool waitForReader, bool waitForWriter, int64_t bufferSize);

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void SetFastMemcpy(bool fastMemcpy);
