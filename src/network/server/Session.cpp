//
//  Session.cpp
//  dirtyserver
//
//  Created by Eugene Sturm on 2/3/19.
//

#include "Session.h"
#include "Connection.h"
#include "World.h"
#include "Player.h"
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
        
        if (_player == nullptr) {
            switch (type) {
            case MessageType::Login: {
                handleLoginMessage(packet);
                break;
            }
            }
        }
        else {
            switch (type) {
            case MessageType::CChat: {
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
    
    _player = new Player(createInfo, this);
    _world->map()->addPlayer(_player);
}

void Session::handleChatMessage(Packet& packet)
{
    std::string contents;
    packet >> contents;
    _world->broadcastPacket(ServerChatMessage(_player->guid(), std::move(contents)));
}

void Session::sendPacket(Packet packet)
{
    _connection->queueOutgoing(std::move(packet));
}
