#pragma once
#pragma comment(lib, "Ws2_32.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <unordered_map>

#include "../Scene/object.h"

static class Network {
public:

    enum NetMsgType {
        MSG_SNAPSHOT,
        MSG_SPAWN,
        MSG_DESPAWN,
        MSG_INPUT,
        MSG_READY,
        MSG_FULL_BEGIN,
        MSG_FULL_END,
		MSG_COUNT // not a real message, just used for bounds checking
    };

    struct netObject {
        uint32_t netID;
        std::string assetID;
        std::shared_ptr<Object::engineObjectBase> obj;
    };

    enum class NetworkEventType
    {
        None = 0,
        ClientConnected,                // server only
        ClientDisconnected,             // server only
        MessageReceived,                // server: from clientSocket. client: from server
        ServerStarted,                  // server only (success)
        ServerStopped,                  // server only
        ClientConnectedToServer,        // client only (success)
        ClientDisconnectedFromServer,   // client only
        Error                           // generic error
    };

    struct DelayedPacket
    {
        SOCKET socket = INVALID_SOCKET;
        std::vector<uint8_t> data;
        clock_t releaseTimeMs = 0;
    };

    struct NetworkEvent
    {
        NetworkEventType type = NetworkEventType::None;
        SOCKET socket = INVALID_SOCKET;
        std::vector<uint8_t> data;
    };

    class NetworkEventQueue
    {
    public:
        void push(NetworkEvent&& event);
        void push(NetworkEventType type, SOCKET serverSocket, const std::string& msg);
        bool poll(NetworkEvent& out);

    private:
        std::mutex mtx;
        std::queue<NetworkEvent> q;
    };

    class WinsockGuard
    {
    public:
        WinsockGuard();
        ~WinsockGuard();
        bool ok() const { return started; }

    private:
        static std::mutex refMtx;
        static int refCount;
        bool started = false;
    };

    struct ClientConnection
    {
        SOCKET socket;
        bool isReady;

        ClientConnection(SOCKET s)
            : socket(s), isReady(false)
        {
        }
    };

    class NetworkServer
    {
    public:
        NetworkServer();
        ~NetworkServer();

        bool start(uint16_t port);
        void stop();
        bool isRunning() const { return running.load(); }
        bool sendToClient(SOCKET client, const void* data, size_t len);
        void broadcast(const void* data, size_t len);
        bool pollEvent(NetworkEvent& out) { return events.poll(out); }
        std::shared_ptr<Network::netObject> registerAndSpawnNetworkObject(std::shared_ptr<Object::engineObjectBase> obj, const std::string& assetID);
        void broadcastDespawn(uint32_t id);
        void auditNetObjects();
        void broadcastSnapshotToAllClients(uint32_t tick);
        void sendFullStateToClient(SOCKET& client);
        void appendToClientBuffer(SOCKET sock, const uint8_t* data, size_t len);
        void enqueueDelayedPacket(SOCKET sock, const std::vector<uint8_t>& payload);
        void processIncomingBuffer(SOCKET sock, std::vector<uint8_t>& buffer);
        void handlePacket(SOCKET sock, const std::vector<uint8_t>& packet);
        void handleInput(SOCKET sock, const std::vector<uint8_t>& packet);
        void processDelayedPackets();
        void consumeInputs();

        std::unordered_map<uint32_t, std::shared_ptr<Network::netObject>> netObjects;
        std::mutex netObjectsMutex;
        std::unordered_map<SOCKET, std::vector<uint8_t>> recvBuffers;
        std::mutex recvBuffersMutex;
        std::unordered_map<SOCKET, std::vector<uint8_t>> latestInputs;
        std::mutex inputMutex;
        std::unordered_map<SOCKET, uint32_t> clientOwnedEntity;
        std::mutex ownershipMutex;

        std::function<void(SOCKET, const std::vector<uint8_t>&)> onMessage;
        std::function<void(SOCKET, const std::vector<uint8_t>&)> onClientInputConsume;
        std::function<void(SOCKET)> onClientConnected;
        std::function<void(SOCKET)> onClientDisconnected;

    private:
        void acceptThreadFunc();
        void clientThreadFunc(SOCKET clientSocket);

        void addClient(SOCKET s);
        void removeClient(SOCKET s);


        WinsockGuard winsock;

        std::atomic<bool> running{ false };
        SOCKET listenSocket = INVALID_SOCKET;


        std::queue<DelayedPacket> delayedPackets;
        std::mutex delayedPacketsMutex;

        std::thread acceptThread;
        std::vector<std::thread> clientThreads;
        std::vector<ClientConnection> clients;
        std::mutex clientsMutex;

        NetworkEventQueue events;
    };

    class NetworkClient
    {
    public:
        NetworkClient();
        ~NetworkClient();

        bool connectTo(const std::string& ip, uint16_t port);
        void disconnect();
        bool sendData(const void* data, size_t len);
        bool sendData(const std::string& msg) {return sendData(msg.data(), msg.size());};
        bool isConnected() const { return connected.load(); }
        bool pollEvent(NetworkEvent& out) { return events.poll(out); }
        void handleNetworkBuffer(const std::vector<uint8_t>& buf);
        void enqueueDelayedPacket(const std::vector<uint8_t>& payload);
        void processDelayedPackets();
        bool sendInput(const std::vector<uint8_t>& payload);

        std::unordered_map<uint32_t, std::shared_ptr<Network::netObject>> netObjects;
        std::function<void(const std::vector<uint8_t>&)> onMessage = nullptr;
        std::mutex netObjectsMutex;
        bool receivingFullState = false;
    private:
        void recvThreadFunc(SOCKET serverSocket);

        WinsockGuard winsock;
        std::atomic<bool> connected{ false };
        SOCKET serverSocket = INVALID_SOCKET;
        std::thread recvThread;

        std::queue<DelayedPacket> delayedPackets;
        std::mutex delayedPacketsMutex;

        NetworkEventQueue events;
    };

    static NetworkServer server;
    static NetworkClient client;
    static int debugClientLagMs;
    static int debugServerLagMs;
    static const float NET_SIM_DT;
};

