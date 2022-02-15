/*
<Brief Description: Header file for NI-RDMA>
Copyright (c) 2021 National Instruments.
All rights reserved.
*/

#ifndef _NI_RDMA_H
#define _NI_RDMA_H

#include <stdint.h>

#ifdef __cplusplus
   extern "C" {
#endif

#if defined(_WIN32)
    #define C_CONV __cdecl
    #if defined(_BUILDING_NIRDMA)
        #define _IMPORT_EXPORT __declspec(dllexport)
    #else
        #define _IMPORT_EXPORT __declspec(dllimport)
    #endif
#else
    #define C_CONV
    #if defined(_BUILDING_NIRDMA)
        #define _IMPORT_EXPORT __attribute__ ((section (".export")))
    #else
        #define _IMPORT_EXPORT
    #endif
#endif
#define  _RDMA_FUNC _IMPORT_EXPORT C_CONV

#pragma pack( push, 8 )

// RDMA Error codes
#define nirdma_Error_Success 0
#define nirdma_Error_Timeout -734001 // 0xFFF4CCCF: Operation timed out.
#define nirdma_Error_InvalidSession -734002 // 0xFFF4CCCE: The specified session could not be found.
#define nirdma_Error_InvalidArgument -734003 // 0xFFF4CCCD: Invalid argument.
#define nirdma_Error_InvalidOperation -734004 // 0xFFF4CCCC: Invalid operation.
#define nirdma_Error_NoBuffersQueued -734005 // 0xFFF4CCCB: No buffers queued.
#define nirdma_Error_OperatingSystemError -734006 // 0xFFF4CCCA: Operating system error.
#define nirdma_Error_InvalidSize -734007 // 0xFFF4CCC9: The provided size was invalid.
#define nirdma_Error_OutOfMemory -734008 // 0xFFF4CCC8: Out of memory.
#define nirdma_Error_InternalError -734009 // 0xFFF4CCC7: An internal error occurred. Contact National Instruments for support.
#define nirdma_Error_InvalidAddress -734010 // 0xFFF4CCC6: Invalid address.
#define nirdma_Error_OperationCancelled -734011 // 0xFFF4CCC5: Operation cancelled.
#define nirdma_Error_InvalidProperty -734012 // 0xFFF4CCC4: Invalid property.
#define nirdma_Error_SessionNotConfigured -734013 // 0xFFF4CCC3: Session not configured.
#define nirdma_Error_NotConnected -734014 // 0xFFF4CCC2: Not connected.
#define nirdma_Error_UnableToConnect -734015 // 0xFFF4CCC1: Unable to connect.
#define nirdma_Error_AlreadyConfigured -734016 // 0xFFF4CCC0: Already configured.
#define nirdma_Error_Disconnected -734017 // 0xFFF4CCBF: Disconnected.
#define nirdma_Error_BufferWaitInProgress -734018 // 0xFFF4CCBE: Blocking buffer operation already in progress.
#define nirdma_Error_AlreadyConnected -734019 // 0xFFF4CCBD: Current session is already connected.
#define nirdma_Error_InvalidDirection -734020 // 0xFFF4CCBC: Specified direction is invalid.
#define nirdma_Error_IncompatibleProtocol -734021 // 0xFFF4CCBB: Incompatible protocol.
#define nirdma_Error_IncompatibleVersion -734022 // 0xFFF4CCBA: Incompatible version.
#define nirdma_Error_ConnectionRefused -734023 // 0xFFF4CCB9: Connection refused.
#define nirdma_Error_ReadOnlyProperty -734024 // 0xFFF4CCB8: Writing a read-only property is not permitted.
#define nirdma_Error_WriteOnlyProperty -734025 // 0xFFF4CCB7: Reading a write-only property is not permitted.
#define nirdma_Error_OperationNotSupported -734026 // 0xFFF4CCB6: The current operation is not supported.
#define nirdma_Error_AddressInUse -734027 // 0xFFF4CCB5: The requested address is already in use.

// Direction used in Connect/Accept
#define nirdma_Direction_Send      0x00
#define nirdma_Direction_Receive   0x01

// Enumeration address type filter
#define nirdma_AddressFamily_AF_UNSPEC  0x00 // Enumerate any address family
#define nirdma_AddressFamily_AF_INET    0x04 // Enumerate only IPv4 interfaces
#define nirdma_AddressFamily_AF_INET6   0x06 // Enumerate only IPv6 interfaces

// Properties
#define nirdma_Property_QueuedBuffers   0x100     // uint64_t
#define nirdma_Property_Connected       0x101     // uint8_t/bool
#define nirdma_Property_UserBuffers     0x102     // uint64_t
#define nirdma_Property_UseRxPolling    0x103     // uint8_t/bool

// Structures
struct nirdma_AddressString {
    char addressString[64];
};

struct nirdma_InternalBufferRegion {
    union {
        struct {
            struct {
                void* buffer;                // Pointer to internally-allocated buffer
                size_t bufferSize;           // Size of internally-allocated buffer
                size_t usedSize;             // Size actually filled (set by API on receive, can be overridden by caller on send)
            };
            struct {
                void* internalReference1;    // Used internally by the API
                void* internalReference2;    // Used internally by the API
            } Internal;
        };
        char padding[64];                    // Ensure struct is large enough for future additions
    };
};

// Callback function for buffer completion
typedef void (C_CONV *nirdma_BufferCompletionCallback)(void* context1, void* context2, int32_t completionStatus, size_t completedBytes);

struct nirdma_BufferCompletionCallbackData {
    nirdma_BufferCompletionCallback callbackFunction = nullptr;
    void* context1 = nullptr;
    void* context2 = nullptr;
};

typedef struct nirdma_Session_struct* nirdma_Session;
#define nirdma_InvalidSession nullptr

int32_t _RDMA_FUNC nirdma_Enumerate(nirdma_AddressString addresses[], size_t* numAddresses, int32_t filterAddressFamily = nirdma_AddressFamily_AF_UNSPEC);
int32_t _RDMA_FUNC nirdma_CreateConnectorSession(const char* localAddress, uint16_t localPort, nirdma_Session* session);
int32_t _RDMA_FUNC nirdma_CreateListenerSession(const char* localAddress, uint16_t localPort, nirdma_Session* session);
int32_t _RDMA_FUNC nirdma_AbortSession(nirdma_Session session);
int32_t _RDMA_FUNC nirdma_CloseSession(nirdma_Session session, uint32_t flags = 0);
int32_t _RDMA_FUNC nirdma_Connect(nirdma_Session connectorSession, uint32_t direction, const char* remoteAddress, uint16_t remotePort, int32_t timeoutMs);
int32_t _RDMA_FUNC nirdma_Accept(nirdma_Session listenSession, uint32_t direction, int32_t timeoutMs, nirdma_Session* connectedSession);
int32_t _RDMA_FUNC nirdma_GetLocalAddress(nirdma_Session session, nirdma_AddressString* localAddress, uint16_t* localPort);
int32_t _RDMA_FUNC nirdma_GetRemoteAddress(nirdma_Session session, nirdma_AddressString* remoteAddress, uint16_t* remotePort);
int32_t _RDMA_FUNC nirdma_ConfigureBuffers(nirdma_Session session, size_t maxTransactionSize, size_t maxConcurrentTransactions);
int32_t _RDMA_FUNC nirdma_ConfigureExternalBuffer(nirdma_Session session, void* externalBuffer, size_t bufferSize, size_t maxConcurrentTransactions);
int32_t _RDMA_FUNC nirdma_AcquireSendRegion(nirdma_Session session, int32_t timeoutMs, nirdma_InternalBufferRegion* bufferRegion);
int32_t _RDMA_FUNC nirdma_AcquireReceivedRegion(nirdma_Session session, int32_t timeoutMs, nirdma_InternalBufferRegion* bufferRegion);
int32_t _RDMA_FUNC nirdma_QueueBufferRegion(nirdma_Session session, nirdma_InternalBufferRegion* bufferRegion, nirdma_BufferCompletionCallbackData* callback);
int32_t _RDMA_FUNC nirdma_QueueExternalBufferRegion(nirdma_Session session, void* pointerWithinBuffer, size_t size, nirdma_BufferCompletionCallbackData* callbackData, int32_t timeoutMs);
int32_t _RDMA_FUNC nirdma_ReleaseReceivedBufferRegion(nirdma_Session session, nirdma_InternalBufferRegion* bufferRegion);
int32_t _RDMA_FUNC nirdma_GetProperty(nirdma_Session session, uint32_t propertyId, void* value, size_t* valueSize);
int32_t _RDMA_FUNC nirdma_SetProperty(nirdma_Session session, uint32_t propertyId, const void* value, size_t valueSize);
int32_t _RDMA_FUNC nirdma_GetLastErrorString(char* buffer, size_t bufferSize);


#pragma pack( pop )

#ifdef __cplusplus
   }
#endif

#endif //_NI_RDMA_H