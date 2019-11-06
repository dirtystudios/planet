//
//  Peer.cpp
//  dirtycommon
//
//  Created by Eugene Sturm on 2/3/19.
//

#include "Peer.hpp"
#include <enet/enet.h>
#include <array>

std::string getAddressString(uint32_t address)
{
    std::array<uint8_t, 4> addrParts {
        (uint8_t)address,
        (uint8_t)(address >> 8),
        (uint8_t)(address >> 16),
        (uint8_t)(address >> 24),
    };
    
    return std::to_string((uint32_t)addrParts[0])
    + "." + std::to_string((uint32_t)addrParts[1])
    + "." + std::to_string((uint32_t)addrParts[2])
    + "." + std::to_string((uint32_t)addrParts[3]);
}

Peer::Peer(ENetPeer* peer)
: _peer(peer)
, _address(getAddressString(_peer->address.host))
{
}

const std::string& Peer::host() const
{
    return _address;
}

uint16_t Peer::port() const
{
    return _peer->address.port;
}
