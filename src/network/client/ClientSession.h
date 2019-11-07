#pragma once

#include <memory>
#include <unordered_map>
#include <functional>
#include "Packet.h"

class World;
class Connection;
using ConnectionPtr = std::shared_ptr<Connection>;

using ClientPacketHandler = std::function<void(Packet)>;
using ClientSessionHandler = std::function<void(Packet)>;

enum class SessionStatus {
    Disconnected,
    Connected,
    Authed,
    LoggedIn,
};

class ClientSession {
    ConnectionPtr _connection{ nullptr };
    SessionStatus _status{ SessionStatus::Disconnected };

    std::unordered_map<MessageType, std::vector<ClientPacketHandler>> _handlers;
    std::unordered_map<SessionStatus, std::vector<ClientSessionHandler>> _sessionhandlers;

public:
    ClientSession() = delete;
    ClientSession(const ConnectionPtr& connection);

    void registerHandler(MessageType type, ClientPacketHandler&& handler);
    void registerHandler(SessionStatus type, ClientSessionHandler&& handler);
    SessionStatus sessionStatus() { return _status; }

    void processIncoming();
};