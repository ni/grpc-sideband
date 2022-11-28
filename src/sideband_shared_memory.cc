//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <cstring>
#include <iostream>
#include <sideband_data.h>
#include <sideband_internal.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
SharedMemorySidebandData::SharedMemorySidebandData(const std::string& id, int64_t bufferSize) :
    SidebandData(bufferSize),
#ifdef _WIN32
    _mapFile(INVALID_HANDLE_VALUE),
#endif
    _buffer(nullptr),
    _usageId(id),
    _id("TESTBUFFER_" + id),
    _bufferSize(bufferSize)
{
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
SharedMemorySidebandData::~SharedMemorySidebandData()
{
#ifdef _WIN32
    UnmapViewOfFile(_buffer);
    CloseHandle(_mapFile);
#endif
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
SharedMemorySidebandData* SharedMemorySidebandData::InitNew(int64_t bufferSize)
{    
    std::string usageId = "TestBuffer";
    auto sidebandData = new SharedMemorySidebandData(usageId, bufferSize);
    return sidebandData;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
const std::string& SharedMemorySidebandData::UsageId()
{    
    return _usageId;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
inline uint8_t* SharedMemorySidebandData::GetBuffer()
{
    if (_buffer == nullptr)
    {
        Init();
    }
    return _buffer;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void SharedMemorySidebandData::Init()
{
#ifdef _WIN32
    _mapFile = CreateFileMappingA(
        INVALID_HANDLE_VALUE,    // use paging file
        NULL,                    // default security
        PAGE_READWRITE,          // read/write access
        0,                       // maximum object size (high-order DWORD)
        _bufferSize,             // maximum object size (low-order DWORD)
        _id.c_str());            // name of mapping object

    if (_mapFile == NULL)
    {
        std::cout << "Could not create file mapping object " << GetLastError() << std::endl;
        return;
    }
    _buffer = (uint8_t*)MapViewOfFile(_mapFile, FILE_MAP_ALL_ACCESS, 0, 0, _bufferSize);
    if (_buffer == NULL)
    {
        std::cout << "Could not map view of file " << GetLastError() << std::endl;
        CloseHandle(_mapFile);
    }
#endif
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool SharedMemorySidebandData::Write(const uint8_t* bytes, int64_t bytecount)
{    
    auto ptr = GetBuffer();
    if (!ptr)
    {
        return false;
    }
    memcpy(ptr, bytes, bytecount);
    return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool SharedMemorySidebandData::Read(uint8_t* bytes, int64_t bufferSize, int64_t* numBytesRead)
{    
    auto ptr = GetBuffer();
    if (!ptr)
    {
        return false;
    }
    memcpy(bytes, ptr, bufferSize);
    return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool SharedMemorySidebandData::WriteLengthPrefixed(const uint8_t* bytes, int64_t byteCount)
{    
    auto ptr = GetBuffer();
    if (!ptr)
    {
        return false;
    }
    *reinterpret_cast<int64_t*>(ptr) = byteCount;
    ptr += sizeof(int64_t);
    memcpy(ptr, bytes, byteCount);
    return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool SharedMemorySidebandData::ReadFromLengthPrefixed(uint8_t* bytes, int64_t bufferSize, int64_t* numBytesRead)
{
    auto ptr = GetBuffer() + sizeof(int64_t);
    if (!ptr)
    {
        return false;
    }
    memcpy(bytes, ptr, bufferSize);
    return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int64_t SharedMemorySidebandData::ReadLengthPrefix()
{
    auto ptr = GetBuffer();
    return *reinterpret_cast<int64_t*>(ptr);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
const uint8_t* SharedMemorySidebandData::BeginDirectRead(int64_t byteCount)
{
    return GetBuffer();    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
const uint8_t* SharedMemorySidebandData::BeginDirectReadLengthPrefixed(int64_t* bufferSize)
{
    auto ptr = GetBuffer();
    if (!ptr)
    {
        return nullptr;
    }
    *bufferSize = *reinterpret_cast<int64_t*>(ptr);
    return ptr + sizeof(int64_t);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool SharedMemorySidebandData::FinishDirectRead()
{
    return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
uint8_t* SharedMemorySidebandData::BeginDirectWrite()
{
    return GetBuffer();
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool SharedMemorySidebandData::FinishDirectWrite(int64_t byteCount)
{
    return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
DoubleBufferedSharedMemorySidebandData::DoubleBufferedSharedMemorySidebandData(const std::string& id, int64_t bufferSize) :
    SidebandData(bufferSize),
    _bufferA(id + "_A", bufferSize),
    _bufferB(id + "_B", bufferSize),
    _id(id)
{
    _current = &_bufferA;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
DoubleBufferedSharedMemorySidebandData::~DoubleBufferedSharedMemorySidebandData()
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
DoubleBufferedSharedMemorySidebandData* DoubleBufferedSharedMemorySidebandData::InitNew(int64_t bufferSize)
{    
    std::string usageId = "TestBuffer";
    auto sidebandData = new DoubleBufferedSharedMemorySidebandData(usageId, bufferSize);
    return sidebandData;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool DoubleBufferedSharedMemorySidebandData::Write(const uint8_t* bytes, int64_t byteCount)
{
    auto result = _current->Write(bytes, byteCount);
    _current = _current == &_bufferA ? &_bufferB : &_bufferA;    
    return result;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool DoubleBufferedSharedMemorySidebandData::Read(uint8_t* bytes, int64_t bufferSize, int64_t* numBytesRead)
{    
    auto result = _current->Read(bytes, bufferSize, numBytesRead);
    _current = _current == &_bufferA ? &_bufferB : &_bufferA;    
    return result;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool DoubleBufferedSharedMemorySidebandData::WriteLengthPrefixed(const uint8_t* bytes, int64_t byteCount)
{    
    auto result = _current->Write(reinterpret_cast<const uint8_t*>(&byteCount), sizeof(int64_t));
    if (!result)
    {
        return false;
    }
    result = _current->Write(bytes, byteCount);
    if (!result)
    {
        return false;
    }
    _current = _current == &_bufferA ? &_bufferB : &_bufferA;
    return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool DoubleBufferedSharedMemorySidebandData::ReadFromLengthPrefixed(uint8_t* bytes, int64_t bufferSize, int64_t* numBytesRead)
{
    auto result = _current->ReadFromLengthPrefixed(bytes, bufferSize, numBytesRead);
    _current = _current == &_bufferA ? &_bufferB : &_bufferA;    
    return result;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int64_t DoubleBufferedSharedMemorySidebandData::ReadLengthPrefix()
{
    return _current->ReadLengthPrefix();
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
const uint8_t* DoubleBufferedSharedMemorySidebandData::BeginDirectRead(int64_t byteCount)
{
    return _current->BeginDirectRead(byteCount);    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
const uint8_t* DoubleBufferedSharedMemorySidebandData::BeginDirectReadLengthPrefixed(int64_t* bufferSize)
{
    return _current->BeginDirectReadLengthPrefixed(bufferSize);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool DoubleBufferedSharedMemorySidebandData::FinishDirectRead()
{    
    _current = _current == &_bufferA ? &_bufferB : &_bufferA;    
    return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
uint8_t* DoubleBufferedSharedMemorySidebandData::BeginDirectWrite()
{
    return _current->BeginDirectWrite();    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool DoubleBufferedSharedMemorySidebandData::FinishDirectWrite(int64_t byteCount)
{    
    _current = _current == &_bufferA ? &_bufferB : &_bufferA;    
    return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
const std::string& DoubleBufferedSharedMemorySidebandData::UsageId()
{
    return _id;
}
