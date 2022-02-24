//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <sstream>
#include <iostream>
#include <cassert>
#include <sys/types.h> 
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#endif

#include <sideband_data.h>
#include <sideband_internal.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif

#ifndef _WIN32
#define SOCKET int
#define INVALID_SOCKET 0
#endif

//---------------------------------------------------------------------
//---------------------------------------------------------------------
Semaphore SocketSidebandData::_connectQueue;
bool SocketSidebandData::_nextConnectLowLatency;
int64_t SocketSidebandData::_nextConnectBufferSize;
std::string SocketSidebandData::_nextConnectionId;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
SocketSidebandData::SocketSidebandData(const std::string& id, int64_t bufferSize, bool lowLatency) :
    SidebandData(bufferSize),
    _socket(INVALID_SOCKET),
    _id(id),
    _lowLatency(lowLatency)
{
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
SocketSidebandData::SocketSidebandData(uint64_t socket, int64_t bufferSize, bool lowLatency) :
    SidebandData(bufferSize),
    _socket(socket),
    _lowLatency(lowLatency)
{
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
SocketSidebandData::~SocketSidebandData()
{    
#ifdef _WIN32
    closesocket(_socket);
#else
    close(_socket);
#endif
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void SocketSidebandData::ConnectToSocket(std::string address, std::string port, std::string usageId, bool lowLatency)
{
#ifdef _WIN32
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo* resultAddress = nullptr;
    auto result = getaddrinfo(address.c_str(), port.c_str(), &hints, &resultAddress);
    if (result != 0)
    {
        std::cout << "getaddrinfo failed with error: " << result << std::endl;
        return;
    }
    for (addrinfo* current = resultAddress; current != nullptr; current = current->ai_next)
    {
        _socket = socket(current->ai_family, SOCK_STREAM, IPPROTO_TCP);
        if (_socket == INVALID_SOCKET)
        {
            std::cout << "socket failed with error: " << WSAGetLastError() << std::endl;
            return;
        }
        result = connect(_socket, current->ai_addr, (int)current->ai_addrlen);
        if (result == SOCKET_ERROR)
        {
            closesocket(_socket);
            _socket = INVALID_SOCKET;
            continue;
        }
        break;
    }
    freeaddrinfo(resultAddress);
    if (_socket == INVALID_SOCKET)
    {
        std::cout << "Unable to connect to server!" << std::endl;
        return;
    }
#else
    int portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    portno = atoi(port.c_str());
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket < 0) 
    {
        std::cout << "ERROR opening socket" << std::endl;
        return;
    }
    server = gethostbyname(address.c_str());
    if (server == NULL)
    {
        std::cout << "ERROR, no such host" << std::endl;
        return;
    }
    memset((char*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    { 
        std::cout << "ERROR connecting" << std::endl;
        return;
    }
#endif

    if (lowLatency)
    {
#ifdef _WIN32
        u_long iMode = 1;
        ioctlsocket(_socket, FIONBIO, &iMode);
#endif
        int yes = 1;
        int result = setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&yes, sizeof(int));
    }

    // Tell the server what shared memory location we are for
    WriteToSocket(const_cast<char*>(usageId.c_str()), usageId.length());
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool SocketSidebandData::WriteToSocket(const void* buffer, int64_t numBytes)
{
    auto remainingBytes = numBytes;
    const char* start = (const char*)buffer;
    while (remainingBytes > 0)
    {
        int written = send(_socket, start, remainingBytes, 0);
        if (written < 0)
        {
            std::cout << "Error writing to buffer";
            return false;
        }
        start += written;
        remainingBytes -= written;
    }
    assert(remainingBytes == 0);
    return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool SocketSidebandData::ReadFromSocket(void* buffer, int64_t numBytes)
{
    auto remainingBytes = numBytes;
    char* start = (char*)buffer;
    while (remainingBytes > 0)
    {        
        int n;
        bool recvAgain = true;
        do
        {
#ifdef _WIN32
            n = recv(_socket, start, remainingBytes, 0);
            auto wsaError = WSAGetLastError();
            recvAgain = n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || wsaError == WSAEWOULDBLOCK);
#else
            n = recv(_socket, start, remainingBytes, 0); // MSG_NOWAIT
            recvAgain = n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK);
#endif
        } while (recvAgain);

        if (n < 0)
        {
            std::cout << "Failed To read." << std::endl;
            return false;
        }
        start += n;
        remainingBytes -= n;
    }
    assert(remainingBytes == 0);
    return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
const std::string& SocketSidebandData::UsageId()
{
    return _id;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
std::vector<std::string> SplitUrlString(const std::string& s)
{
    char delimiter = ':';
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void SocketSidebandData::ReadConnectionId()
{    
    std::vector<char> buffer(ConnectIdLength());
    ReadFromSocket(buffer.data(), ConnectIdLength());
    _id = std::string(buffer.data(), ConnectIdLength());
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void SocketSidebandData::QueueSidebandConnection(::SidebandStrategy strategy, const std::string& id, int64_t bufferSize)
{
    _connectQueue.wait();
#ifdef _WIN32
    _nextConnectLowLatency = false;
#else
    _nextConnectLowLatency = strategy == ::SidebandStrategy::SOCKETS_LOW_LATENCY;
#endif
    _nextConnectBufferSize = bufferSize;
    _nextConnectionId = id;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
SocketSidebandData* SocketSidebandData::InitFromConnection(int socket)
{
    if (_nextConnectLowLatency)
    {
#ifdef _WIN32
        u_long iMode = 1;
        ioctlsocket(socket, FIONBIO, &iMode);
#endif
        int yes = 1;
        int result = setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (char*)&yes, sizeof(int));
    }
    auto sidebandData = new SocketSidebandData(socket, _nextConnectBufferSize, _nextConnectLowLatency);
    sidebandData->ReadConnectionId();
    RegisterSidebandData(sidebandData);
    _connectQueue.notify();
    return sidebandData;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
SocketSidebandData* SocketSidebandData::ClientInit(const std::string& sidebandServiceUrl, const std::string& usageId, int64_t bufferSize, bool lowLatency)
{    
    auto tokens = SplitUrlString(sidebandServiceUrl);
    auto sidebandData = new SocketSidebandData(usageId, bufferSize, lowLatency);
    sidebandData->ConnectToSocket(tokens[0], tokens[1], usageId, lowLatency);
    return sidebandData;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool SocketSidebandData::Write(const uint8_t* bytes, int64_t byteCount)
{
    return WriteToSocket(bytes, byteCount);    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool SocketSidebandData::Read(uint8_t* bytes, int64_t bufferSize, int64_t* numBytesRead)
{
    if (ReadFromSocket(bytes, bufferSize))
    {
        *numBytesRead = bufferSize;
        return true;
    }
    return false;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool SocketSidebandData::WriteLengthPrefixed(const uint8_t* bytes, int64_t byteCount)
{
    auto result = WriteToSocket(&byteCount, sizeof(int64_t));    
    if (result)
    {
        result = WriteToSocket(bytes, byteCount);    
    }
    return result;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool SocketSidebandData::ReadFromLengthPrefixed(uint8_t* bytes, int64_t bufferSize, int64_t* numBytesRead)
{
    return Read(bytes, bufferSize, numBytesRead);    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int64_t SocketSidebandData::ReadLengthPrefix()
{
    int64_t bufferSize = 0;
    ReadFromSocket(&bufferSize, sizeof(int64_t));
    return bufferSize;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
std::string GetSocketsAddress()
{
    auto rdmaAddress = GetRdmaAddress();
    if (rdmaAddress.length() > 0)
    {
        return rdmaAddress;
    }
    return "localhost";
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int RunSidebandSocketsAccept(int port)
{
    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr;

#ifdef _WIN32
    WSADATA wsaData {};
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

    // create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
       std::cout << "ERROR opening socket" << std::endl;
       return -1;
    }

    memset((char *) &serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_addr.s_addr = INADDR_ANY;  
    serv_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
       std::cout << "ERROR on binding" << std::endl;
       return -1;
    }

    listen(sockfd, 5);

    while (true)
    {
        sockaddr_in cli_addr {};
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
        { 
            std::cout << "ERROR on accept" << std::endl;
            return -1;
        }
        std::cout << "Connection!" << std::endl;

        SocketSidebandData::InitFromConnection(newsockfd);
    }
#ifdef _WIN32
    closesocket(sockfd);
#else
    close(sockfd);
#endif
    return 0; 
}
