//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

#include <data_moniker.pb.h>
#include <sideband_data.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
inline int64_t InitMonikerSidebandData(const ni::data_monikers::BeginMonikerSidebandStreamResponse& initResponse)
{
    int64_t token;
    InitClientSidebandData(initResponse.connection_url().c_str(), (::SidebandStrategy)initResponse.strategy(), initResponse.sideband_identifier().c_str(), initResponse.buffer_size(), &token);
    return token;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
inline int64_t InitClientSidebandData(const ni::data_monikers::BeginMonikerSidebandStreamResponse& response)
{
    int64_t token;
    InitClientSidebandData(response.connection_url().c_str(), (::SidebandStrategy)response.strategy(), response.sideband_identifier().c_str(), response.buffer_size(), &token);
    return token;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
inline bool ReadSidebandMessage(int64_t dataToken, google::protobuf::MessageLite* message)
{    
    bool success = false;
    if (SidebandData_SupportsDirectReadWrite(dataToken) == 1)
    {
        int64_t bufferSize = 0;
        const uint8_t* buffer = nullptr;
        SidebandData_BeginDirectReadLengthPrefixed(dataToken, &bufferSize, &buffer);
        success = message->ParseFromArray(buffer, bufferSize);
        SidebandData_FinishDirectRead(dataToken);
    }
    else
    {
        int64_t bufferSize = 0;
        SidebandData_ReadLengthPrefix(dataToken, &bufferSize);
        int64_t bytesRead = 0;
        uint8_t* buffer = nullptr;
        SidebandData_SerializeBuffer(dataToken, &buffer);
        SidebandData_ReadFromLengthPrefixed(dataToken, buffer, bufferSize, &bytesRead);
        success = message->ParseFromArray(buffer, bufferSize);
        assert(success);
    }
    return success;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
inline int64_t WriteSidebandMessage(int64_t dataToken, const google::protobuf::MessageLite& message)
{
    auto byteSize = message.ByteSizeLong();
    if (SidebandData_SupportsDirectReadWrite(dataToken) == 1)
    {
        uint8_t* buffer = nullptr;
        SidebandData_BeginDirectWrite(dataToken, &buffer);
        message.SerializeToArray(buffer, byteSize);
        SidebandData_FinishDirectWrite(dataToken, byteSize);
    }
    else
    {
        uint8_t* buffer = nullptr;
        SidebandData_SerializeBuffer(dataToken, &buffer);
        message.SerializeToArray(buffer, byteSize);
        SidebandData_WriteLengthPrefixed(dataToken, buffer, byteSize);
    }
    return byteSize;
}
