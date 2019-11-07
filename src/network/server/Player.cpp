//
//  Player.cpp
//  dirtyserver
//
//  Created by Eugene Sturm on 2/5/19.
//

#include "Player.h"

Player::Player(const PlayerCreateInfo& createInfo, Session* session)
: _session(session)
, _name(createInfo.name)
, _guid(createInfo.guid)
{
    
}
