//
//  World.cpp
//  dirtyserver
//
//  Created by Eugene Sturm on 2/3/19.
//

#include "World.hpp"
#include "Session.hpp"

World::World()
{
    _map.reset(new Map());
}

void World::addSession(const SessionPtr& session)
{
    session->setWorld(this);
    _sessions.emplace_back(session);
}

void World::update()
{
    std::vector<Packet> packets;
    for (SessionPtr& session : _sessions) {
        session->processIncoming();
    }
}

void World::broadcastPacket(Packet&& packet)
{
    Packet p = std::move(packet);
    for (SessionPtr& session : _sessions) {
        Packet copy = p;
        session->sendPacket(std::move(copy));
    }
}
