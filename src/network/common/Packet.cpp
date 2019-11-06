//
//  Packet.cpp
//  dirtycommon
//
//  Created by Eugene Sturm on 1/29/19.
//

#include "Packet.hpp"

Packet::Packet(const uint8_t* data, size_t len)
{
    write(data, len);
}

Packet::Packet()
{
    
}

Packet::~Packet()
{}

MessageType Packet::messageType() const
{
    const MessageType type = *((MessageType*)data());
    return type;
}
