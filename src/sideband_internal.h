//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include "sideband_semaphore.h"
#include <vector>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class SidebandData
{
public:
    SidebandData(int64_t bufferSize);
    virtual ~SidebandData();
    virtual const std::string& UsageId() = 0;
    virtual bool Write(const uint8_t* bytes, int64_t byteCount) = 0;
    virtual bool Read(uint8_t* bytes, int64_t bufferSize, int64_t* numBytesRead) = 0;
    virtual bool WriteLengthPrefixed(const uint8_t* bytes, int64_t byteCount) = 0;
    virtual bool ReadFromLengthPrefixed(uint8_t* bytes, int64_t bufferSize, int64_t* numBytesRead) = 0;
    virtual int64_t ReadLengthPrefix() = 0;

    virtual bool SupportsDirectReadWrite() { return false; }
    virtual const uint8_t* BeginDirectRead(int64_t byteCount) { return nullptr; }
    virtual const uint8_t* BeginDirectReadLengthPrefixed(int64_t* bufferSize) { return nullptr; }
    virtual bool FinishDirectRead() { return false; }
    virtual uint8_t* BeginDirectWrite() { return nullptr; }
    virtual bool FinishDirectWrite(int64_t byteCount) { return false; }

    uint8_t* SerializeBuffer();

private:
    int64_t _bufferSize;
    std::vector<uint8_t> _serializeBuffer;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class SharedMemorySidebandData : public SidebandData
{
public:
    SharedMemorySidebandData(const std::string& id, int64_t bufferSize);
    virtual ~SharedMemorySidebandData();

    bool Write(const uint8_t* bytes, int64_t byteCount) override;
    bool Read(uint8_t* bytes, int64_t bufferSize, int64_t* numBytesRead) override;
    bool WriteLengthPrefixed(const uint8_t* bytes, int64_t byteCount) override;
    bool ReadFromLengthPrefixed(uint8_t* bytes, int64_t bufferSize, int64_t* numBytesRead) override;
    int64_t ReadLengthPrefix() override;

    bool SupportsDirectReadWrite() override { return true; }
    const uint8_t* BeginDirectRead(int64_t byteCount) override;
    const uint8_t* BeginDirectReadLengthPrefixed(int64_t* bufferSize) override;
    bool FinishDirectRead() override;
    uint8_t* BeginDirectWrite() override;
    bool FinishDirectWrite(int64_t byteCount) override;

    const std::string& UsageId() override;
    uint8_t* GetBuffer();

public:
    static SharedMemorySidebandData* InitNew(int64_t bufferSize);

private:
    void Init();
#ifdef _WIN32
    HANDLE _mapFile;
#else
    int _mapFD;
    std::string _fileName;
#endif
    uint8_t* _buffer;
    std::string _id;
    std::string _usageId;
    int64_t _bufferSize;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class DoubleBufferedSharedMemorySidebandData : public SidebandData
{
public:
    DoubleBufferedSharedMemorySidebandData(const std::string& id, int64_t bufferSize);
    virtual ~DoubleBufferedSharedMemorySidebandData();

    bool Write(const uint8_t* bytes, int64_t byteCount) override;
    bool Read(uint8_t* bytes, int64_t bufferSize, int64_t* numBytesRead) override;
    bool WriteLengthPrefixed(const uint8_t* bytes, int64_t byteCount) override;
    bool ReadFromLengthPrefixed(uint8_t* bytes, int64_t bufferSize, int64_t* numBytesRead) override;
    int64_t ReadLengthPrefix() override;

    bool SupportsDirectReadWrite() override { return true; }
    const uint8_t* BeginDirectRead(int64_t byteCount) override;
    const uint8_t* BeginDirectReadLengthPrefixed(int64_t* bufferSize) override;
    bool FinishDirectRead() override;
    uint8_t* BeginDirectWrite() override;
    bool FinishDirectWrite(int64_t byteCount) override;

    const std::string& UsageId() override;

public:
    static DoubleBufferedSharedMemorySidebandData* InitNew(int64_t bufferSize);

private:
    std::string _id;
    SharedMemorySidebandData* _current;
    SharedMemorySidebandData _bufferA;
    SharedMemorySidebandData _bufferB;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class SocketSidebandData : public SidebandData
{
public:
    SocketSidebandData(const std::string& id, int64_t bufferSize, bool lowLatency);
    SocketSidebandData(uint64_t socket, int64_t bufferSize, bool lowLatency);
    virtual ~SocketSidebandData();

    bool Write(const uint8_t* bytes, int64_t byteCount) override;
    bool Read(uint8_t* bytes, int64_t bufferSize, int64_t* numBytesRead) override;
    bool WriteLengthPrefixed(const uint8_t* bytes, int64_t byteCount) override;
    bool ReadFromLengthPrefixed(uint8_t* bytes, int64_t bufferSize, int64_t* numBytesRead) override;
    int64_t ReadLengthPrefix() override;

    const std::string& UsageId() override;

public:
    static void QueueSidebandConnection(::SidebandStrategy strategy, const std::string& id, int64_t bufferSize);
    static SocketSidebandData* InitFromConnection(int socket);
    static SocketSidebandData* ClientInit(const std::string& sidebandServiceUrl, const std::string& usageId, int64_t bufferSize, bool lowLatency);

private:
    void ReadConnectionId();
    void ConnectToSocket(std::string address, std::string port, std::string usageId, bool lowLatency);
    bool WriteToSocket(const void* buffer, int64_t numBytes);
    bool ReadFromSocket(void* buffer, int64_t numBytes);

private:
    std::string _id;
    uint64_t _socket;
    bool _lowLatency;

private:
    static Semaphore _connectQueue;
    static bool _nextConnectLowLatency;
    static int64_t _nextConnectBufferSize;
    static std::string _nextConnectionId;
};

#ifdef ENABLE_RDMA_SIDEBAND

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class RdmaSidebandDataImp;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class RdmaSidebandData : public SidebandData
{
public:
    RdmaSidebandData(const std::string& id, RdmaSidebandDataImp* implementation);
    virtual ~RdmaSidebandData();
    const std::string& UsageId() override;

    bool Write(const uint8_t* bytes, int64_t byteCount) override;
    bool Read(uint8_t* bytes, int64_t bufferSize, int64_t* numBytesRead) override;
    bool WriteLengthPrefixed(const uint8_t* bytes, int64_t byteCount) override;
    bool ReadFromLengthPrefixed(uint8_t* bytes, int64_t bufferSize, int64_t* numBytesRead) override;
    int64_t ReadLengthPrefix() override;

    bool SupportsDirectReadWrite() override { return true; }
    const uint8_t* BeginDirectRead(int64_t byteCount) override;
    const uint8_t* BeginDirectReadLengthPrefixed(int64_t* bufferSize) override;
    bool FinishDirectRead() override;
    uint8_t* BeginDirectWrite() override;
    bool FinishDirectWrite(int64_t byteCount) override;
public:
    static void QueueSidebandConnection(::SidebandStrategy strategy, const std::string& id, bool waitForReader, bool waitForWriter, int64_t bufferSize);
    static RdmaSidebandData* ClientInit(const std::string& sidebandServiceUrl, bool lowLatency, const std::string& usageId, int64_t bufferSize);

private:
    std::unique_ptr<RdmaSidebandDataImp> _imp;
    std::string _id;
};
#endif

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void RegisterSidebandData(SidebandData* sidebandData);

std::vector<std::string> SplitUrlString(const std::string& s);
int ConnectIdLength();
std::string NextConnectionId();

//---------------------------------------------------------------------
//---------------------------------------------------------------------
std::string GetConnectionAddress(::SidebandStrategy strategy);

#ifdef ENABLE_RDMA_SIDEBAND
std::string GetRdmaAddress();
#endif

std::string GetSocketsAddress();
std::string GetSocketsPort();
