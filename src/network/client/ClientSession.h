#pragma once

#include <memory>
#include <unordered_map>
#include <functional>
#include "Packet.h"
#include "Connection.h"

class Socket;

using ClientPacketHandler = std::function<void(Packet&)>;
using ClientSessionHandler = std::function<void()>;

enum class SessionStatus {
    Disconnected,
    Connecting,
    Connected,
    Authed,
    LoggedIn,
};

class ClientSession {
    ConnectionPtr _connection{ nullptr };
    Socket* _socket{ nullptr };
    SessionStatus _status{ SessionStatus::Disconnected };

    std::unordered_map<MessageType, std::vector<ClientPacketHandler>> _handlers;
    std::unordered_map<SessionStatus, std::vector<ClientSessionHandler>> _sessionhandlers;

    uint64_t _guid{ 0 };

public:
    ClientSession() = delete;
    ClientSession(const ConnectionPtr& connection);

    void registerHandler(MessageType type, ClientPacketHandler&& handler);
    void registerHandler(SessionStatus type, ClientSessionHandler&& handler);
    SessionStatus sessionStatus() { return _status; }
    uint64_t guid() {return _guid; }

    void processIncoming();

    template<typename T> 
    void queueOutgoing(const T& message) {
        _connection->queueOutgoing(Packet(message));
    }

private:
    void setStatus(SessionStatus status);
    void handlePacket(MessageType type, Packet& p);
};