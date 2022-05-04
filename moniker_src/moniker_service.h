//---------------------------------------------------------------------
// Implementation of MonikerService
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpcpp/grpcpp.h>
#include <data_moniker.grpc.pb.h>
#include <type_traits>
#include <map>
#include <sideband_data.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
namespace ni
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    using MonikerEndpointPtr = std::add_pointer<grpc::Status(void*, google::protobuf::Any&)>::type;
    using EndpointInstance = std::tuple<MonikerEndpointPtr, void*>;
    using EndpointList = std::vector<EndpointInstance>;

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class MonikerServiceImpl final : public ni::data_monikers::MonikerService::Service
    {
    public:
        MonikerServiceImpl();
        grpc::Status BeginSidebandStream(grpc::ServerContext* context, const ni::data_monikers::BeginMonikerSidebandStreamRequest* request, ni::data_monikers::BeginMonikerSidebandStreamResponse* response) override;
        grpc::Status StreamReadWrite(grpc::ServerContext* context, grpc::ServerReaderWriter<::ni::data_monikers::MonikerReadResult, ni::data_monikers::MonikerWriteRequest>* stream) override;
        grpc::Status StreamRead(grpc::ServerContext* context, const ni::data_monikers::MonikerList* request, grpc::ServerWriter<ni::data_monikers::MonikerReadResult>* writer);
        grpc::Status StreamWrite(grpc::ServerContext* context, grpc::ServerReaderWriter<ni::data_monikers::StreamWriteResponse, ni::data_monikers::MonikerWriteRequest>* stream);

    public:
        static void RegisterMonikerEndpoint(std::string endpointName, MonikerEndpointPtr endpoint);
        static void RegisterMonikerInstance(std::string endpointName, void* instanceData, ni::data_monikers::Moniker& moniker);
    
    private:
        static MonikerServiceImpl* s_Server;
        std::map<std::string, MonikerEndpointPtr> _endpoints;

    private:
        void InitiateMonikerList(const ni::data_monikers::MonikerList& monikers, EndpointList& readers, EndpointList& writers);
        static void RunSidebandReadWriteLoop(std::string sidebandIdentifier, ::SidebandStrategy strategy, EndpointList& readers, EndpointList& writers, bool initialClientWrite);
    };
}
