//
//  World.cpp
//  dirtyserver
//
//  Created by Eugene Sturm on 2/3/19.
//

#include "World.h"
#include "Session.h"

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

void World::broadcastPacket(const Packet& packet)
{
    for (SessionPtr& session : _sessions) {
        session->sendPacket(packet);
    }
}
