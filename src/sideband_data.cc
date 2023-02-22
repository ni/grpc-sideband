//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <map>
#include <cassert>
#include <iostream>
#include <atomic>
#include <cstring>
#include "sideband_data.h"
#include "sideband_internal.h"

//---------------------------------------------------------------------
//---------------------------------------------------------------------
std::mutex _bufferLockMutex;
std::condition_variable _bufferLock;
std::map<std::string, SidebandData*> _buffers;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
SidebandData::SidebandData(int64_t bufferSize) :
    _bufferSize(bufferSize)
{
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
SidebandData::~SidebandData()
{
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
uint8_t* SidebandData::SerializeBuffer()
{
    if (_serializeBuffer.size() == 0)
    {
        _serializeBuffer = std::vector<uint8_t>(_bufferSize);
    }
    return _serializeBuffer.data();
}


std::atomic_int _nextId;
std::string _zeroId = NextConnectionId();

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int ConnectIdLength()
{
    return _zeroId.length();
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
std::string NextConnectionId()
{
    char buffer[20];
    auto id = ++_nextId;
    sprintf(buffer, "ID:%10d", id);
    return buffer;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC SidebandData_Write(int64_t sidebandToken, const uint8_t* bytes, int64_t byteCount)
{
    auto sidebandData = reinterpret_cast<SidebandData*>(sidebandToken);
    auto result = sidebandData->Write(bytes, byteCount);
    return result ? 0 : -1;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC SidebandData_Read(int64_t sidebandToken, uint8_t* bytes, int64_t bufferSize, int64_t* numBytesRead)
{
    auto sidebandData = reinterpret_cast<SidebandData*>(sidebandToken);
    auto result = sidebandData->Read(bytes, bufferSize, numBytesRead);
    return result ? 0 : -1;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC SidebandData_WriteLengthPrefixed(int64_t sidebandToken, const uint8_t* bytes, int64_t byteCount)
{
    auto sidebandData = reinterpret_cast<SidebandData*>(sidebandToken);
    auto result = sidebandData->WriteLengthPrefixed(bytes, byteCount);
    return result ? 0 : -1;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC SidebandData_ReadFromLengthPrefixed(int64_t sidebandToken, uint8_t* bytes, int64_t bufferSize, int64_t* numBytesRead)
{
    auto sidebandData = reinterpret_cast<SidebandData*>(sidebandToken);
    auto result = sidebandData->ReadFromLengthPrefixed(bytes, bufferSize, numBytesRead);
    return result ? 0 : -1;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC SidebandData_ReadLengthPrefix(int64_t sidebandToken, int64_t* length)
{
    auto sidebandData = reinterpret_cast<SidebandData*>(sidebandToken);
    *length = sidebandData->ReadLengthPrefix();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC SidebandData_SupportsDirectReadWrite(int64_t sidebandToken)
{
    auto sidebandData = reinterpret_cast<SidebandData*>(sidebandToken);
    auto result = sidebandData->SupportsDirectReadWrite();
    return result ? 1 : 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC SidebandData_BeginDirectRead(int64_t sidebandToken, int64_t byteCount, const uint8_t** buffer)
{
    auto sidebandData = reinterpret_cast<SidebandData*>(sidebandToken);
    *buffer = sidebandData->BeginDirectRead(byteCount);
    return *buffer != nullptr ? 0 : -1;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC SidebandData_BeginDirectReadLengthPrefixed(int64_t sidebandToken, int64_t* bufferSize, const uint8_t** buffer)
{
    auto sidebandData = reinterpret_cast<SidebandData*>(sidebandToken);
    *buffer = sidebandData->BeginDirectReadLengthPrefixed(bufferSize);
    return *buffer != nullptr ? 0 : -1;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC SidebandData_FinishDirectRead(int64_t sidebandToken)
{
    auto sidebandData = reinterpret_cast<SidebandData*>(sidebandToken);
    auto result = sidebandData->FinishDirectRead();
    return result ? 0 : -1;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC SidebandData_BeginDirectWrite(int64_t sidebandToken, uint8_t** buffer)
{
    auto sidebandData = reinterpret_cast<SidebandData*>(sidebandToken);
    *buffer = sidebandData->BeginDirectWrite();
    return *buffer != nullptr ? 0 : -1;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC SidebandData_FinishDirectWrite(int64_t sidebandToken, int64_t byteCount)
{
    auto sidebandData = reinterpret_cast<SidebandData*>(sidebandToken);
    auto result = sidebandData->FinishDirectWrite(byteCount);
    return result ? 0 : -1;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC SidebandData_SerializeBuffer(int64_t sidebandToken, uint8_t** buffer)
{
    auto sidebandData = reinterpret_cast<SidebandData*>(sidebandToken);
    *buffer = sidebandData->SerializeBuffer();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC QueueSidebandConnection(::SidebandStrategy strategy, const char* id, bool waitForReader, bool waitForWriter, int64_t bufferSize)
{
    switch (strategy)
    {
    case ::SidebandStrategy::RDMA:
    case ::SidebandStrategy::RDMA_LOW_LATENCY:
        RdmaSidebandData::QueueSidebandConnection(strategy, id, waitForReader, waitForWriter, bufferSize);
        break;
    case ::SidebandStrategy::SOCKETS:
    case ::SidebandStrategy::SOCKETS_LOW_LATENCY:
        SocketSidebandData::QueueSidebandConnection(strategy, id, bufferSize);
    default:
        // don't need to queue for non RDMA strategies
        return 0;
    }
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC InitOwnerSidebandData(::SidebandStrategy strategy, int64_t bufferSize, char out_sideband_id[32])
{
    std::unique_lock<std::mutex> lock(_bufferLockMutex);
    switch (strategy)
    {
        case ::SidebandStrategy::DOUBLE_BUFFERED_SHARED_MEMORY:
            {
                auto sidebandData = DoubleBufferedSharedMemorySidebandData::InitNew(bufferSize);
                _buffers.emplace(sidebandData->UsageId(), sidebandData);
                strcpy(out_sideband_id, sidebandData->UsageId().c_str());
                return 0;
            }
            break;
        case ::SidebandStrategy::SHARED_MEMORY:
            {
                auto sidebandData = SharedMemorySidebandData::InitNew(bufferSize);
                _buffers.emplace(sidebandData->UsageId(), sidebandData);
                strcpy(out_sideband_id, sidebandData->UsageId().c_str());
                return 0;
            }
            break;
        case ::SidebandStrategy::SOCKETS:
        case ::SidebandStrategy::SOCKETS_LOW_LATENCY:
            strcpy(out_sideband_id, NextConnectionId().c_str());
            return 0;
        case ::SidebandStrategy::RDMA:
        case ::SidebandStrategy::RDMA_LOW_LATENCY:
            strcpy(out_sideband_id, NextConnectionId().c_str());
            return 0;
    }
    assert(false);
    return -1;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC GetOwnerSidebandDataToken(const char* usageId, int64_t* out_tokenId)
{
    std::unique_lock<std::mutex> lock(_bufferLockMutex);

    while (_buffers.find(usageId) == _buffers.end()) _bufferLock.wait(lock);
    *out_tokenId = reinterpret_cast<int64_t>(_buffers[usageId]);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC InitClientSidebandData(const char* sidebandServiceUrl, ::SidebandStrategy strategy, const char* usageId, int bufferSize, int64_t* out_tokenId)
{
    SidebandData* sidebandData = nullptr;
    bool insert = true;
    switch (strategy)
    {
        case ::SidebandStrategy::SHARED_MEMORY:
            sidebandData = new SharedMemorySidebandData(usageId, bufferSize);
            break;
        case ::SidebandStrategy::DOUBLE_BUFFERED_SHARED_MEMORY:
            sidebandData = new DoubleBufferedSharedMemorySidebandData(usageId, bufferSize);
            break;
        case ::SidebandStrategy::SOCKETS:
        case ::SidebandStrategy::SOCKETS_LOW_LATENCY:
            sidebandData = SocketSidebandData::ClientInit(sidebandServiceUrl, usageId, bufferSize, strategy == ::SidebandStrategy::SOCKETS_LOW_LATENCY);
            break;
        case ::SidebandStrategy::RDMA:
            sidebandData = RdmaSidebandData::ClientInit(sidebandServiceUrl, false, usageId, bufferSize);
            insert = false;
            break;
        case ::SidebandStrategy::RDMA_LOW_LATENCY:
            sidebandData = RdmaSidebandData::ClientInit(sidebandServiceUrl, true, usageId, bufferSize);
            insert = false;
            break;
    }
    if (insert)
    {
        std::unique_lock<std::mutex> lock(_bufferLockMutex);

        assert(_buffers.find(usageId) == _buffers.end());
        _buffers.emplace(usageId, sidebandData);
    }
    *out_tokenId = reinterpret_cast<int64_t>(sidebandData);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void RegisterSidebandData(SidebandData* sidebandData)
{
    std::unique_lock<std::mutex> lock(_bufferLockMutex);

    assert(_buffers.find(sidebandData->UsageId()) == _buffers.end());
    _buffers.emplace(sidebandData->UsageId(), sidebandData);

    _bufferLock.notify_all();
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC WriteSidebandData(int64_t dataToken, uint8_t* bytes, int64_t bytecount)
{
    auto sidebandData = reinterpret_cast<SidebandData*>(dataToken);
    sidebandData->Write(bytes, bytecount);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC ReadSidebandData(int64_t dataToken, uint8_t* bytes, int64_t bufferSize, int64_t* numBytesRead)
{
    auto sidebandData = reinterpret_cast<SidebandData*>(dataToken);
    sidebandData->Read(bytes, bufferSize, numBytesRead);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC CloseSidebandData(int64_t dataToken)
{
    std::unique_lock<std::mutex> lock(_bufferLockMutex);

    auto sidebandData = reinterpret_cast<SidebandData*>(dataToken);
    assert(_buffers.find(sidebandData->UsageId()) != _buffers.end());
    _buffers.erase(sidebandData->UsageId());
    delete sidebandData;
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
std::string GetConnectionAddress(::SidebandStrategy strategy)
{
    std::string address;
    if (strategy == ::SidebandStrategy::RDMA ||
        strategy == ::SidebandStrategy::RDMA_LOW_LATENCY)
    {
        address = GetRdmaAddress() + ":50060";
    }
    else
    {
        address = GetSocketsAddress() + ":50055";
    }
    std::cout << "Connection address: " << address << std::endl;
    return address;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t _SIDEBAND_FUNC GetSidebandConnectionAddress(::SidebandStrategy strategy, char address[1024])
{
    sprintf(address, GetConnectionAddress(strategy).c_str());
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
extern "C" __declspec(dllexport) BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpReserved)  // reserved
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
