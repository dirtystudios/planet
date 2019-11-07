#pragma once

#include "Socket.h"
#include "Connection.h"
#include "World.h"
#include "Session.h"

class Server {
private:
    std::mutex enteringConnectionsMutex;
    std::vector<ConnectionPtr> enteringConnections;

    std::mutex exitingConnectionsMutex;
    std::vector<ConnectionPtr> exitingConnections;

    std::vector<ConnectionPtr> pendingConnections;
    std::vector<ConnectionPtr> activeConnections;
    World world;

public:
    Server();

    void processIncoming();
};