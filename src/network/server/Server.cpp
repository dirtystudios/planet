#include "Server.h"
#include "Log.h"

Server::Server() {
    auto eventDelegate = [&](SocketEventType type, ConnectionPtr& connection)
    {
        switch (type) {
        case SocketEventType::PeerConnected: {
            std::lock_guard<std::mutex> lock(enteringConnectionsMutex);
            enteringConnections.emplace_back(connection);
            break;
        }
        case SocketEventType::PeerDisconnected: {
            std::lock_guard<std::mutex> lock(exitingConnectionsMutex);
            exitingConnections.emplace_back(connection);
            break;
        }
        default: {
            break;
        }
        }
    };

    socket = std::make_unique<Socket>(44951, eventDelegate);
}

void Server::processIncoming() {
    {
        std::lock_guard<std::mutex> lock(enteringConnectionsMutex);
        pendingConnections.insert(end(pendingConnections), begin(enteringConnections), end(enteringConnections));
        enteringConnections.clear();
    }

    Packet packet;
    for (auto it = begin(pendingConnections); it != end(pendingConnections);) {
        auto& connection = *it;

        bool didAuth = false;
        std::optional<Packet> dequeueResult;
        while ((dequeueResult = connection->dequeueIncoming()).has_value()) {
            Packet& packet = dequeueResult.value();

            const MessageType type = packet.messageType();
            if (type == MessageType::Auth) {
                connection->queueOutgoing(Packet(AuthResponseMessage()));
                didAuth = true;
                break;
            }
            else {
                LOG_E("server dropping %s", to_string(type));
            }
        }

        if (didAuth) {
            it = pendingConnections.erase(it);
            activeConnections.emplace_back(connection);
            world.addSession(std::make_shared<Session>(connection));
        }
        else {
            ++it;
        }
    }
}

void Server::update() {
    world.update();
}