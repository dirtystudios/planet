//
//  Session.hpp
//  dirtyserver
//
//  Created by Eugene Sturm on 2/3/19.
//

#pragma once

#include <memory>
#include "Packet.hpp"

class World;
class Connection;
using ConnectionPtr = std::shared_ptr<Connection>;

class Session
{
private:
    ConnectionPtr _connection { nullptr };
    World* _world { nullptr };
public:
    Session(const ConnectionPtr& connection);
    
    void setWorld(World* world);
    
    void processIncoming();    
    void sendPacket(Packet packet);
protected:
    void handleChatMessage(Packet& packet);
    void handleLoginMessage(Packet& packet);
    void handleMoveMessage(Packet& packet);
};
