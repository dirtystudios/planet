//
//  Session.cpp
//  dirtyserver
//
//  Created by Eugene Sturm on 2/3/19.
//

#include "Session.hpp"
#include "Connection.hpp"
#include "World.hpp"
#include "Player.hpp"
#include "GuidProvider.h"

Session::Session(const ConnectionPtr& connection)
: _connection(connection)
{
}

void Session::setWorld(World* world)
{
    _world = world;
}

void Session::processIncoming()
{
    std::optional<Packet> packetResult;
    while ((packetResult = _connection->dequeueIncoming()).has_value()) {
        Packet& packet = packetResult.value();
        const MessageType type = packet.read<MessageType>();
        
        
        switch (type) {
            case MessageType::Login: {
                handleLoginMessage(packet);
                break;
            }
            case MessageType::SChat: {
                handleChatMessage(packet);
                break;
            }
            case MessageType::Move: {
                handleMoveMessage(packet);
                break;
            }
            default: {
                break;
            }
        }
    }
}

void Session::handleMoveMessage(Packet& packet)
{
    
}

void Session::handleLoginMessage(Packet& packet)
{
    std::string name;
    packet >> name;
      
    PlayerCreateInfo createInfo;
    createInfo.guid = Guid::generateGuid();
    createInfo.name = name;
    
    Packet response;
    response << MessageType::LoginResponse;
    sendPacket(std::move(response));
    
    Player* player = new Player(createInfo, this);
    _world->map()->addPlayer(player);
}

void Session::handleChatMessage(Packet& packet)
{
    Packet copy = packet;
    _world->broadcastPacket(std::move(copy));
}

void Session::sendPacket(Packet packet)
{
    _connection->queueOutgoing(std::move(packet));
}
