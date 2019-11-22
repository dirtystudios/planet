//
//  Map.cpp
//  dirtyserver
//
//  Created by Eugene Sturm on 2/5/19.
//

#include "Map.h"
#include "Player.h"
#include "Session.h"

Map::Map()
{
    
}

void Map::addPlayer(Player* player)
{
    if (_players.insert(player).second == false) {
        return;
    }

    // send information of everything in Map to player
    for (Player* otherPlayer : _players) {
        //if (otherPlayer == player) { continue; }

        auto p = Packet(ServerObjectMessage(otherPlayer->guid(), std::string(otherPlayer->name())));
        player->session()->sendPacket(p);
    }
    
    // send information of new player to everyone on 
    auto p = Packet(ServerObjectMessage(player->guid(), std::string(player->name())));;

    auto exclusionPredicate = [player](Player* otherPlayer) -> bool { return otherPlayer == player; };
    broadcastPacket(p, exclusionPredicate);
}

void Map::broadcastPacket(const Packet& packet, std::function<bool(Player*)> exclusionPredicate)
{
    for (Player* player : _players) {
        if (exclusionPredicate(player)) { continue; }
        player->session()->sendPacket(packet);
    }
}
