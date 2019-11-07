//
//  Packet.hpp
//  dirtycommon
//
//  Created by Eugene Sturm on 1/29/19.
//

#pragma once

#include "MessageType.h"
#include "ByteStream.h"

class Packet : public ByteStream
{
public:
    template <typename T>
    Packet(const T& msg)
    {
        msg.pack(*this);
    }
    
    Packet();
    
    Packet(const uint8_t* data, size_t len);    
    ~Packet();
    
    MessageType messageType() const;
};


