# gRPC sideband data support

## Overview
the gRPC sideband library is intended to be used in conjunction with a standard gRPC service to provide low latency or high bandwidth communication between the client and server.  In most cases gRPC streaming support is sufficient but in cases where large amounts of data (>1-2GB/s) needs to be streamed, or the communication needs to happen with very low latency (<30us).

The sideband channel supports a variety of methods for communication:
* SHARED_MEMORY - use a shared memory region for communcation.  This works when both the client and the server are on the same computer.
* DOUBLE_BUFFERED_SHARED_MEMORY - Same as shared memory but with a double buffered memory region so that the server can write the next set of data while the client is reading.
* SOCKETS - uses a standard TCP socket to stream the data.  this is especially useful for testing an implementation since it uses standard hardware.
* SOCKETS_LOW_LATENCY - uses a standard TCP socket that has been configured for low latency use. This will comsume more CPU than a standard socket setup.
* HYPERVISOR_SOCKETS - Not currently implemented.
* RDMA - Uses RDMA to perform the data transfer.  Network cards must both support RDMA.
* RDMA_LOW_LATENCY - Uses RDMA with a low latency communcation model for the data transfer. This will consume more CPU than standard RDMA communication.

This project supports Windows, Linux and Linux RT.

---
## Examples
The [gprc-perf](https://github.com/ni/grpc-perf) performance testing repo contains examples on how you can incorporate sideband communication into your gRPC based API.
### Example 1 - Low Latency Communication
The grpc-perf performance testing repo has a simple example of how to integrate side-band communication with a gRPC API.
The purpose of this example is use the sideband channel for 2-way low latency communication.

Client implementation is in [PerformSidebandMonikerLatencyTest](https://github.com/ni/grpc-perf/blob/606ec6d8fe37ebafe12cd8502885caef179c829b/src/performance_tests.cc#L114)

The server code which implements the initiation of the streaming in [NIPerfTestServer::BeginTestSidebandStream](https://github.com/ni/grpc-perf/blob/606ec6d8fe37ebafe12cd8502885caef179c829b/src/perftest_server.cc#L197)

The client will then use the Data Moniker service to performance the read-write loop which is implemented by the server in [ NIMonikerServer::BeginSidebandStream](https://github.com/ni/grpc-perf/blob/606ec6d8fe37ebafe12cd8502885caef179c829b/src/perftest_server.cc#L299)

The read write loop is implemented by the server in [NIMonikerServer::BeginSidebandStream](https://github.com/ni/grpc-perf/blob/606ec6d8fe37ebafe12cd8502885caef179c829b/src/perftest_server.cc#L299)


### Example 2 - High bandwidth communication
In this example the clinet and server perform a streaming communication call using a standard gRPC bidirectional stream.  A sideband channel is established to transfer large payload data as part of the call.

Client code is in [PerformSidebandReadTest](https://github.com/ni/grpc-perf/blob/606ec6d8fe37ebafe12cd8502885caef179c829b/src/performance_tests.cc#L187)

Server code is in [ NIPerfTestServer::TestSidebandStream](https://github.com/ni/grpc-perf/blob/606ec6d8fe37ebafe12cd8502885caef179c829b/src/perftest_server.cc#L211)

---
## Building
See [Building](docs/building.md)
