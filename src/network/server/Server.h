#pragma once

#include "Socket.h"
#include "Connection.h"
#include "World.h"
#include "Session.h"
#include <memory>

class Server {
private:
    std::mutex enteringConnectionsMutex;
    std::vector<ConnectionPtr> enteringConnections;

    std::mutex exitingConnectionsMutex;
    std::vector<ConnectionPtr> exitingConnections;

    std::vector<ConnectionPtr> pendingConnections;
    std::vector<ConnectionPtr> activeConnections;
    World world;

    std::unique_ptr<Socket> socket;

public:
    Server();

    void processIncoming();

    void update();
};