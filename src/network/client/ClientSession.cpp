#include "ClientSession.h"
#include "Log.h"
#include "Connection.h"
#include "DGAssert.h"

ClientSession::ClientSession(const ConnectionPtr& connection)
:_connection(connection) {

}

void ClientSession::processIncoming() {
    std::vector<Packet> packets;
    _connection->drainIncomingQueue(&packets);

    for (Packet& packet : packets) {
        const MessageType type = packet.read<MessageType>();

        LOG_D("ClientSession Recv: %s", to_string(type));

        switch (type) {
        case MessageType::AuthResponse: {
            AuthResponseMessage a;
            a.unpack(packet);
            _guid = a.sessionId;
            handlePacket(type, packet);
            setStatus(SessionStatus::Authed);
            break;
        }
        case MessageType::LoginResponse: {
            handlePacket(type, packet);
            setStatus(SessionStatus::LoggedIn);
            break;
        }
        case MessageType::SChat:
        case MessageType::SObject: {
            handlePacket(type, packet);
            break;
        }
        default: {
            break;
        }
        }
    }

    _connection->flushOutgoingQueue();
}

void ClientSession::setStatus(SessionStatus status) {
    _status = status;
    auto check = _sessionhandlers.find(status);
    if (check != _sessionhandlers.end()) {
        for (auto& h : check->second) {
            h();
        }
    }
}

void ClientSession::handlePacket(MessageType type, Packet& p) {
    auto check = _handlers.find(type);
    if (check != _handlers.end()) {
        for (auto& h : check->second) {
            h(p);
        }
    }
}

void ClientSession::registerHandler(MessageType type, ClientPacketHandler&& handler) {
    auto check = _handlers.find(type);
    if (check != _handlers.end()) {
        check->second.emplace_back(std::move(handler));
    }
    else
        _handlers[type] = std::vector<ClientPacketHandler>{ std::move(handler) };
}

void ClientSession::registerHandler(SessionStatus type, ClientSessionHandler&& handler) {
    auto check = _sessionhandlers.find(type);
    if (check != _sessionhandlers.end()) {
        check->second.emplace_back(std::move(handler));
    }
    else
        _sessionhandlers[type] = std::vector<ClientSessionHandler>{ std::move(handler) };
}