//
//  Peer.hpp
//  dirtycommon
//
//  Created by Eugene Sturm on 2/3/19.
//

#pragma once

#include <string>

class _ENetPeer;

class Peer
{
private:
    _ENetPeer* _peer { nullptr };
    std::string _address;
public:
    Peer(_ENetPeer* peer);
    
    const std::string& host() const;
    uint16_t port() const;        
};
