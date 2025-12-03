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
#include <unordered_map> // added

#include "../Scene/object.h"

static class Network {
public:

    enum class NetMsgType : uint8_t {
        MSG_SNAPSHOT = 1,
        MSG_SPAWN = 3,
        MSG_DESPAWN = 2,
    };

    struct netObject {
        uint32_t netID;
        //tick rate?
        std::shared_ptr<Object::engineObject> obj;
        std::string texturePath;//get rid of and come up with a better solution, premade objects?
    };

    enum class NetworkEventType
    {
        None = 0,
        ClientConnected,                // server only
        ClientDisconnected,             // server only
        MessageReceived,                // server: from clientSocket; client: from server
        ServerStarted,                  // server only (success)
        ServerStopped,                  // server only
        ClientConnectedToServer,        // client only (success)
        ClientDisconnectedFromServer,   // client only
        Error                           // generic error (use message)
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
        void push(NetworkEventType type, SOCKET sock, const std::string& msg);
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

    class NetworkServer
    {
    public:
        NetworkServer();
        ~NetworkServer();

        // Start listening on the given port (returns true on success)
        bool start(uint16_t port);

        // Stop listening and shutdown all clients (blocks until threads clean up)
        void stop();

        bool isRunning() const { return running.load(); }

        // Send a string to a single client (returns false on failure)
        bool sendToClient(SOCKET client, const void* data, size_t len);

        // Broadcast to all connected clients
        void broadcast(const void* data, size_t len);

        // Poll one network event (non-blocking)
        bool pollEvent(NetworkEvent& out) { return events.poll(out); }

        void broadcastSnapshotToAllClients(uint32_t tick);
        std::shared_ptr<Network::netObject> registerAndSpawnNetworkObject(std::shared_ptr<Object::engineObject> obj, const std::string& texture_path);
        void broadcastDespawn(uint32_t id);
        void auditNetObjects();

        std::unordered_map<uint32_t, std::shared_ptr<Network::netObject>> netObjects;
        std::mutex netObjects_mtx;

    private:
        void acceptThreadFunc();
        void clientThreadFunc(SOCKET clientSocket);

        void addClient(SOCKET s);
        void removeClient(SOCKET s);

        WinsockGuard winsock;

        std::atomic<bool> running{ false };
        SOCKET listenSocket = INVALID_SOCKET;

        std::thread acceptThread;
        std::vector<std::thread> clientThreads;
        std::vector<SOCKET> clients;
        std::mutex clientsMutex;

        NetworkEventQueue events;
    };

    class NetworkClient
    {
    public:
        NetworkClient();
        ~NetworkClient();

        // Connect to server (returns true on success)
        bool connectTo(const std::string& ip, uint16_t port);

        // Disconnect from server (blocks until thread exits)
        void disconnect();

        // Send a message to the connected server
        bool sendData(const void* data, size_t len);
        bool sendData(const std::string& msg) {return sendData(msg.data(), msg.size());};

        bool isConnected() const { return connected.load(); }

        // Poll one network event (non-blocking)
        bool pollEvent(NetworkEvent& out) { return events.poll(out); }

        void handleNetworkBuffer(const std::vector<uint8_t>& buf);

        std::unordered_map<uint32_t, std::shared_ptr<Network::netObject>> netObjects;
        std::mutex netObjects_mtx;
    private:
        void recvThreadFunc(SOCKET sock);

        std::function<void(const std::vector<uint8_t>&)> onMessage = nullptr;
        WinsockGuard winsock;
        std::atomic<bool> connected{ false };
        SOCKET sock = INVALID_SOCKET;
        std::thread recvThread;

        NetworkEventQueue events;
    };

    static NetworkServer server;
    static NetworkClient client;
   
};