#include "network.h"
#include <cstdio>
#include <algorithm>
#include <chrono>
#include <cstdarg>
#include "../Core/logger.h"
#include "../InputOutput/serialization.h"
#include "../Graphics/texture.h"
#include "../Scene/scene.h"
#include "../Scene/asset.h"


Network::NetworkServer Network::server;
Network::NetworkClient Network::client;
uint32_t nextNetID = 1; // increment for new network objects
static const size_t MAX_EVENTS = 5000;


//NetworkEventQueue
void Network::NetworkEventQueue::push(NetworkEvent&& event)
{
    std::lock_guard<std::mutex> lock(mtx);

    q.push(std::move(event));

    // Auto-trim if nobody is polling
    if (q.size() > MAX_EVENTS) {
        size_t toRemove = q.size() - MAX_EVENTS;
        while (toRemove-- > 0) {
            q.pop(); // discard oldest events
        }
    }
}

void Network::NetworkEventQueue::push(NetworkEventType type, SOCKET sock, const std::string& msg)
{
    NetworkEvent event{ type, sock, {} };
    event.data.assign(msg.begin(), msg.end());

    push(std::move(event));
}

bool Network::NetworkEventQueue::poll(NetworkEvent& out)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (q.empty()) return false;
    out = std::move(q.front());
    q.pop();
    return true;
}

//WinsockGuard
std::mutex Network::WinsockGuard::refMtx;
int Network::WinsockGuard::refCount = 0;

Network::WinsockGuard::WinsockGuard()
{
    std::lock_guard<std::mutex> lock(refMtx);
    if (refCount == 0) {
        WSADATA wsaData;
        int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0) {
            Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug, "[Net] WSAStartup failed: %d", iResult);
            started = false;
        }
        else {
            started = true;
            Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,"[Net] Winsock initialized.");
        }
    }
    else
    { 
        refCount++;
        started = true;
    }
}

Network::WinsockGuard::~WinsockGuard()
{
    std::lock_guard<std::mutex> lock(refMtx);
    refCount--;
    if (refCount == 0 && started) {
        WSACleanup();
        Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,"[Net] Winsock cleaned up.");
        started = false;
    }
}

//NetworkServer
Network::NetworkServer::NetworkServer()
    : winsock()
{
}

Network::NetworkServer::~NetworkServer()
{
    stop();
}

bool Network::NetworkServer::start(uint16_t port)
{
    if (!winsock.ok()) {
        Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,"Server winsock not initialized.");
        return false;
    }

    if (running.load()) return false;

    // prepare addrinfo
    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    char portStr[8];
    snprintf(portStr, sizeof(portStr), "%u", port);

    addrinfo* result = nullptr;
    int iResult = getaddrinfo(NULL, portStr, &hints, &result);
    if (iResult != 0) {
        Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,"[Net][Server] getaddrinfo failed: %d", iResult);
        events.push( NetworkEventType::Error, INVALID_SOCKET, "getaddrinfo failed" );
        return false;
    }

    listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listenSocket == INVALID_SOCKET) {
        Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,"[Net][Server] socket failed: %d", WSAGetLastError());
        freeaddrinfo(result);
        events.push( NetworkEventType::Error, INVALID_SOCKET, "socket failed" );
        return false;
    }

    iResult = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,"[Net][Server] bind failed: %d", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        events.push( NetworkEventType::Error, INVALID_SOCKET, "bind failed" );
        return false;
    }

    freeaddrinfo(result);

    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,"[Net][Server] listen failed: %d", WSAGetLastError());
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        events.push( NetworkEventType::Error, INVALID_SOCKET, "listen failed" );
        return false;
    }

    running = true;
    acceptThread = std::thread(&NetworkServer::acceptThreadFunc, this);
    Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,"[Net][Server] Started on port %u", port);
    events.push( NetworkEventType::ServerStarted, INVALID_SOCKET, "Server started" );
    return true;
}

void Network::NetworkServer::stop()
{
    if (!running.load()) return;

    running = false;

    // cause accept to break
    if (listenSocket != INVALID_SOCKET) {
        shutdown(listenSocket, SD_BOTH);
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
    }

    // close all client sockets
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (SOCKET s : clients) {
            shutdown(s, SD_BOTH);
            closesocket(s);
        }
        clients.clear();
    }

    // join accept thread
    if (acceptThread.joinable()) acceptThread.join();

    // join client threads
    for (auto& t : clientThreads) {
        if (t.joinable()) t.join();
    }
    clientThreads.clear();

    events.push( NetworkEventType::ServerStopped, INVALID_SOCKET, "Server stopped" );
    Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,"[Net][Server] Stopped.");
}

void Network::NetworkServer::acceptThreadFunc()
{
    while (running) {
        Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,"[Net][Server] Waiting for accept...");
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            int err = WSAGetLastError();
            if (!running) break; // shutdown requested
            Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,"[Net][Server] accept failed: %d", err);
            events.push( NetworkEventType::Error, INVALID_SOCKET, "accept failed" );
            // small sleep to avoid tight loop on continuous errors
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        addClient(clientSocket);
        events.push( NetworkEventType::ClientConnected, clientSocket, "Client connected" );
        Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,"[Net][Server] Client connected: %d", int(clientSocket));

        // spawn client thread
        clientThreads.emplace_back(&NetworkServer::clientThreadFunc, this, clientSocket);
    }
    Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,"[Net][Server] Accept thread exiting.");
}

void Network::NetworkServer::clientThreadFunc(SOCKET clientSocket)
{
    const int bufLen = 512;
    char buf[bufLen];
    bool connectedLocal = true;

    while (connectedLocal && running) {
        int iResult = recv(clientSocket, buf, bufLen, 0);
        if (iResult > 0) {
            std::string msg(buf, iResult);
            events.push( NetworkEventType::MessageReceived, clientSocket, std::move(msg) );
        }
        else if (iResult == 0) {
            // client closed
            connectedLocal = false;
            events.push( NetworkEventType::ClientDisconnected, clientSocket, "Client closed connection" );
            Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,"[Net][Server] Client %d closed connection", int(clientSocket));
            break;
        }
        else {
            int err = WSAGetLastError();
            if (err == WSAEINTR || err == WSAENOTSOCK || !running) {
                // likely from shutdown, treat as disconnected
                connectedLocal = false;
                events.push( NetworkEventType::ClientDisconnected, clientSocket, "Client disconnected (error/shutdown)" );
                break;
            }
            Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,"[Net][Server] recv failed: %d", err);
            events.push( NetworkEventType::Error, clientSocket, "recv failed" );
            connectedLocal = false;
            break;
        }
    }

    // cleanup this client
    removeClient(clientSocket);
    shutdown(clientSocket, SD_BOTH);
    closesocket(clientSocket);
    Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,"[Net][Server] Client %d cleaned up.", int(clientSocket));
}

void Network::NetworkServer::addClient(SOCKET s)
{
    std::lock_guard<std::mutex> lock(clientsMutex);
    clients.push_back(s);
}

void Network::NetworkServer::removeClient(SOCKET s)
{
    std::lock_guard<std::mutex> lock(clientsMutex);
    clients.erase(std::remove(clients.begin(), clients.end(), s), clients.end());
}

bool Network::NetworkServer::sendToClient(SOCKET client, const void* data, size_t len)
{
    // Build [length|payload]
    uint32_t packetLen = (uint32_t)len;
    std::vector<uint8_t> buffer;
    buffer.reserve(sizeof(packetLen) + len);

    // prepend length in network byte order
    uint32_t netLen = htonl(packetLen);
    buffer.insert(buffer.end(),
        reinterpret_cast<uint8_t*>(&netLen),
        reinterpret_cast<uint8_t*>(&netLen) + sizeof(netLen));

    // append payload
    buffer.insert(buffer.end(),
        reinterpret_cast<const uint8_t*>(data),
        reinterpret_cast<const uint8_t*>(data) + len);

    int iResult = send(client, reinterpret_cast<const char*>(buffer.data()),
        (int)buffer.size(), 0);

    if (iResult == SOCKET_ERROR) {
        Logger::log(Logger::LogCategory::General, Logger::LogLevel::Error,
            "[Net][Server] send failed for %d: %d", int(client), WSAGetLastError());
        events.push(NetworkEventType::Error, client, "send failed");
        return false;
    }

    return true;
}

void Network::NetworkServer::broadcast(const void* data, size_t len)
{
    std::vector<SOCKET> snapshot;
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        snapshot = clients;
    }

    std::vector<SOCKET> failed;
    for (SOCKET s : snapshot) {
        if (!sendToClient(s, data, len))
            failed.push_back(s);
    }

    if (!failed.empty()) {
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (SOCKET s : failed) {
            removeClient(s);
        }
    }
}

std::shared_ptr<Network::netObject> Network::NetworkServer::registerAndSpawnNetworkObject( std::shared_ptr<Object::engineObjectBase> obj, const std::string& assetID) {
    uint32_t ID = nextNetID++;

    std::shared_ptr<Network::netObject> netObj;

    {
        std::lock_guard<std::mutex> lk(netObjects_mtx);

        // Create and assign
        netObj = std::make_shared<netObject>();
        netObj->netID = ID;
        netObj->assetID = assetID;
        netObj->obj = obj;

        // Store it in the map
        netObjects[ID] = netObj;
    }

    if (ID == 0) {//bad method?
        Logger::log(Logger::LogCategory::General, Logger::LogLevel::Error,
            "Server Failed to register network object.");
        return nullptr;
    }

    // Build spawn message
    std::vector<uint8_t> buf;
    Serialization::append_uint8(buf, static_cast<uint8_t>(Network::NetMsgType::MSG_SPAWN));
    Serialization::append_u32(buf, ID);
    Serialization::append_string(buf, assetID);

    // Initial transform state
    Serialization::append_float(buf, obj->hull.x);
    Serialization::append_float(buf, obj->hull.y);
    Serialization::append_float(buf, obj->hull.w);
    Serialization::append_float(buf, obj->hull.h);
    Serialization::append_double(buf, obj->rot);
    Serialization::append_i32(buf, obj->texIndex);
    Serialization::append_float(buf, obj->scale);
    Serialization::append_uint8(buf, static_cast<uint8_t>(obj->flip));
    Serialization::append_i32(buf, obj->depth);

    broadcast(buf.data(), buf.size());

    return netObj;
}

void Network::NetworkServer::broadcastDespawn(uint32_t id)
{
    if (id == 0) { // <=0?
        Logger::log(Logger::LogCategory::Network, Logger::LogLevel::Warn,
            "Server Tried to despawn object with invalid ID.");
        return;
    }

    // Remove from server-side first
    {
        std::lock_guard<std::mutex> lk(netObjects_mtx);
        auto it = netObjects.find(id);
        if (it != netObjects.end()) {
            netObjects.erase(it);
        }
        else {
            Logger::log(Logger::LogCategory::Network, Logger::LogLevel::Warn,
                "Server Tried to despawn unknown netID %u", id);
            return;
        }
    }

    // Build despawn message
    std::vector<uint8_t> buf;
    Serialization::append_uint8(buf, static_cast<uint8_t>(Network::NetMsgType::MSG_DESPAWN));
    Serialization::append_u32(buf, id);

    // Broadcast to all clients (size header added internally)
    broadcast(buf.data(), buf.size());

    Logger::log(Logger::LogCategory::Network, Logger::LogLevel::Trace,
        "Server broadcasted despawn for ID %u", id);
}

void Network::NetworkServer::auditNetObjects()
{
    //look for netObject flagged of removal
    std::vector<uint32_t> toRemove;
    for (const auto& pair : netObjects) {
        const auto& netObj = pair.second;
        if (netObj->obj->remove) {
            toRemove.push_back(pair.first);
        }
    }
    for (uint32_t id : toRemove) {
		broadcastDespawn(id);
        Logger::log(Logger::LogCategory::General, Logger::LogLevel::Trace, "Server removed netObject ID %u", id);
    }
}

void Network::NetworkServer::broadcastSnapshotToAllClients(uint32_t tick) {
    auditNetObjects();
    
    std::vector<uint8_t> buf;
    Serialization::append_uint8(buf, static_cast<uint8_t>(NetMsgType::MSG_SNAPSHOT));
    Serialization::append_u32(buf, tick);

    // copy snapshot content into temp buffer to determine count
    std::vector<uint8_t> body;
    uint32_t count = 0;

    std::lock_guard<std::mutex> lk(netObjects_mtx);
    for (auto& p : netObjects) {
        auto& obj = p.second->obj;
        Serialization::append_u32(body, p.first); // id
        Serialization::append_float(body, obj->hull.x);
        Serialization::append_float(body, obj->hull.y);
        Serialization::append_float(body, obj->hull.w);
        Serialization::append_float(body, obj->hull.h);
        Serialization::append_double(body, obj->rot);
        Serialization::append_i32(body, obj->texIndex);
        Serialization::append_float(body, obj->scale);
        Serialization::append_uint8(body, static_cast<uint8_t>(obj->flip));
        Serialization::append_i32(body, obj->depth);
        ++count;
    }

    Serialization::append_u32(buf, count);
    buf.insert(buf.end(), body.begin(), body.end());

    // send buf to every connected client using your TCP send
    Logger::log(Logger::LogCategory::Network, Logger::LogLevel::Trace, "Sending snapshot: count = %u, bytes = %zu", count, buf.size());
    broadcast(buf.data(), buf.size());
}

//NetworkClient
Network::NetworkClient::NetworkClient()
    : winsock()
{
    onMessage = [this](const std::vector<uint8_t>& data) {
        this->handleNetworkBuffer(data);
        };
}

Network::NetworkClient::~NetworkClient()
{
    disconnect();
}

bool Network::NetworkClient::connectTo(const std::string& ip, uint16_t port)
{
    if (!winsock.ok()) {
        Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,"Client winsock not initialized.");
        return false;
    }

    if (connected.load()) return false;

    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    char portStr[8];
    snprintf(portStr, sizeof(portStr), "%u", port);

    addrinfo* result = nullptr;
    int iResult = getaddrinfo(ip.c_str(), portStr, &hints, &result);
    if (iResult != 0) {
        Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,"Client getaddrinfo failed: %d", iResult);
        events.push( NetworkEventType::Error, INVALID_SOCKET, "getaddrinfo failed" );
        return false;
    }

    SOCKET connectSocket = INVALID_SOCKET;
    for (addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
        connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (connectSocket == INVALID_SOCKET) continue;

        iResult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(connectSocket);
            connectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }
    freeaddrinfo(result);

    if (connectSocket == INVALID_SOCKET) {
        Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,"Client unable to connect to %s:%u", ip.c_str(), port);
        events.push( NetworkEventType::Error, INVALID_SOCKET, "connect failed" );
        return false;
    }

    sock = connectSocket;
    connected = true;
    // spawn recv thread
    recvThread = std::thread(&NetworkClient::recvThreadFunc, this, sock);
    Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,"Client connected to %s:%u (socket %d)", ip.c_str(), port, int(sock));
    events.push( NetworkEventType::ClientConnectedToServer, sock, "Connected to server" );
    return true;
}

void Network::NetworkClient::disconnect()
{
    if (!connected.load()) return;

    connected = false;
    if (sock != INVALID_SOCKET) {
        shutdown(sock, SD_BOTH);
        closesocket(sock);
        sock = INVALID_SOCKET;
    }

    if (recvThread.joinable()) recvThread.join();
    Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,"Client disconnected.");
    events.push( NetworkEventType::ClientDisconnectedFromServer, INVALID_SOCKET, "Disconnected" );
}

bool Network::NetworkClient::sendData(const void* data, size_t len)
{
    if (!connected.load() || sock == INVALID_SOCKET)
        return false;

    uint32_t packetLen = (uint32_t)len;
    std::vector<uint8_t> buffer;
    buffer.reserve(sizeof(packetLen) + len);

    uint32_t netLen = htonl(packetLen);
    buffer.insert(buffer.end(),
        reinterpret_cast<uint8_t*>(&netLen),
        reinterpret_cast<uint8_t*>(&netLen) + sizeof(netLen));

    buffer.insert(buffer.end(),
        reinterpret_cast<const uint8_t*>(data),
        reinterpret_cast<const uint8_t*>(data) + len);

    int result = send(sock, reinterpret_cast<const char*>(buffer.data()),
        (int)buffer.size(), 0);

    if (result == SOCKET_ERROR) {
        Logger::log(Logger::LogCategory::General, Logger::LogLevel::Error,
            "Client send failed: %d", WSAGetLastError());
        events.push(NetworkEventType::Error, sock, "send failed");
        return false;
    }
    return true;
}

void Network::NetworkClient::handleNetworkBuffer(const std::vector<uint8_t>& buf) {
    size_t idx = 0;
    NetMsgType type = static_cast<NetMsgType>(Serialization::read_u8(buf, idx));

    if (type == NetMsgType::MSG_SNAPSHOT) {
        uint32_t tick = Serialization::read_u32(buf, idx);
        uint32_t num = Serialization::read_u32(buf, idx);
        for (uint32_t i = 0; i < num; ++i) {
            uint32_t id = Serialization::read_u32(buf, idx);
            float x = Serialization::read_float(buf, idx);
            float y = Serialization::read_float(buf, idx);
            float w = Serialization::read_float(buf, idx);
            float h = Serialization::read_float(buf, idx);
            double rot = Serialization::read_double(buf, idx);
            int32_t texIndex = Serialization::read_i32(buf, idx);
            float scale = Serialization::read_float(buf, idx);
            uint8_t flip = Serialization::read_u8(buf, idx);
            int32_t depth = Serialization::read_i32(buf, idx);

            std::lock_guard<std::mutex> lk(netObjects_mtx);
            auto it = netObjects.find(id);
            if (it != netObjects.end()) {
                auto g = it->second->obj;
                g->hull.x = x;
                g->hull.y = y;
                g->hull.w = w;
                g->hull.h = h;
                g->rot = rot;
                g->texIndex = texIndex;
                g->scale = scale;
                g->flip = static_cast<SDL_FlipMode>(flip);
                g->depth = depth;
            }
            else {
                // object not found
                // request spawn? or ignore?
            }
        }
    }
    else if (type == NetMsgType::MSG_SPAWN) {
        uint32_t id = Serialization::read_u32(buf, idx);
        std::string assetID = Serialization::read_string(buf, idx);

        float x = Serialization::read_float(buf, idx);
        float y = Serialization::read_float(buf, idx);
        float w = Serialization::read_float(buf, idx);
        float h = Serialization::read_float(buf, idx);
        double rot = Serialization::read_double(buf, idx);
        int32_t texIndex = Serialization::read_i32(buf, idx);
        float scale = Serialization::read_float(buf, idx);
        uint8_t flip = Serialization::read_u8(buf, idx);
        int32_t depth = Serialization::read_i32(buf, idx);

        // Create ghost
        auto ghost = Asset::load(assetID);

		// Disable update flag for networked objects
        ghost->updateFlag = false;

		// Set initial state
        ghost->hull.x = x;
		ghost->hull.y = y;
		ghost->hull.w = w;
		ghost->hull.h = h;
		ghost->rot = rot;
        ghost->texIndex = texIndex;
		ghost->scale = scale;
		ghost->flip = static_cast<SDL_FlipMode>(flip);
		ghost->depth = depth;
        
        {
            std::lock_guard<std::mutex> lk(netObjects_mtx);
            netObjects[id] = std::make_unique<Network::netObject>();
			netObjects[id]->netID = id;
            netObjects[id]->obj = ghost;
			netObjects[id]->assetID = assetID;
        }
        Scene::addObject(ghost);
    }
    else if (type == NetMsgType::MSG_DESPAWN) {
        uint32_t id = Serialization::read_u32(buf, idx);
        std::shared_ptr<Object::engineObjectBase> toRemove;
        {
            std::lock_guard<std::mutex> lk(netObjects_mtx);
            auto it = netObjects.find(id);
            if (it != netObjects.end()) {
                toRemove = it->second->obj;
                netObjects.erase(it);
            }
        }
        if (toRemove) toRemove->remove = true;
    }
}

void Network::NetworkClient::recvThreadFunc(SOCKET inSock)
{
    std::vector<uint8_t> buffer;
    buffer.reserve(1024);
    uint32_t expectedSize = 0;
    size_t received = 0;

    while (connected.load()) {

        // Read message length
        if (expectedSize == 0) {
            uint32_t netLen;
            int bytes = recv(inSock, (char*)&netLen, sizeof(netLen), MSG_WAITALL);

            if (bytes <= 0) goto disconnect;

            expectedSize = ntohl(netLen);
            buffer.resize(expectedSize);
            received = 0;
        }

        // Read message payload
        int bytes = recv(inSock, (char*)buffer.data() + received,
            expectedSize - received, MSG_WAITALL);

        if (bytes <= 0) goto disconnect;

        received += bytes;

        // Complete message
        if (received == expectedSize) {
            if (onMessage) {
                onMessage(buffer);  // automatic behavior can be swapped out or dissabled
            }

            // Always push events (for custom or extended handling)
            events.push({ NetworkEventType::MessageReceived, inSock, buffer });
            expectedSize = 0; // reset
        }
    }

disconnect:
    connected = false;
    events.push(NetworkEventType::ClientDisconnectedFromServer, inSock, "Connection closed");
    shutdown(inSock, SD_BOTH);
    closesocket(inSock);
}
