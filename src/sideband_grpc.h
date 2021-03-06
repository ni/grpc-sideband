//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

#include <data_moniker.pb.h>
#include <sideband_data.h>
#include <sideband_internal.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
inline int64_t InitMonikerSidebandData(const ni::data_monikers::BeginMonikerSidebandStreamResponse& initResponse)
{
    return InitClientSidebandData(initResponse.connection_url(), (::SidebandStrategy)initResponse.strategy(), initResponse.sideband_identifier(), initResponse.buffer_size());
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
inline int64_t InitClientSidebandData(const ni::data_monikers::BeginMonikerSidebandStreamResponse& response)
{
    return InitClientSidebandData(response.connection_url(), (::SidebandStrategy)response.strategy(), response.sideband_identifier(), response.buffer_size());    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
inline bool ReadSidebandMessage(int64_t dataToken, google::protobuf::MessageLite* message)
{    
    auto sidebandData = reinterpret_cast<SidebandData*>(dataToken);
    bool success = false;
    if (sidebandData->SupportsDirectReadWrite())
    {
        int64_t bufferSize = 0;
        auto buffer = sidebandData->BeginDirectReadLengthPrefixed(&bufferSize);
        success = message->ParseFromArray(buffer, bufferSize);
        sidebandData->FinishDirectRead();
    }
    else
    {
        auto bufferSize = sidebandData->ReadLengthPrefix();
        int64_t bytesRead = 0;
        sidebandData->ReadFromLengthPrefixed(sidebandData->SerializeBuffer(), bufferSize, &bytesRead);
        success = message->ParseFromArray(sidebandData->SerializeBuffer(), bufferSize);
        assert(success);
    }
    return success;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
inline int64_t WriteSidebandMessage(int64_t dataToken, const google::protobuf::MessageLite& message)
{
    auto sidebandData = reinterpret_cast<SidebandData*>(dataToken);
    auto byteSize = message.ByteSizeLong();
    if (sidebandData->SupportsDirectReadWrite())
    {
        auto buffer = sidebandData->BeginDirectWrite();
        message.SerializeToArray(buffer, byteSize);
        sidebandData->FinishDirectWrite(byteSize);
    }
    else
    {
        message.SerializeToArray(sidebandData->SerializeBuffer(), byteSize);
        sidebandData->WriteLengthPrefixed(sidebandData->SerializeBuffer(), byteSize);
    }
    return byteSize;
}
