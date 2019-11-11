//
//  MessageType.hpp
//  dirtycommon
//
//  Created by Eugene Sturm on 1/26/19.
//

#pragma once

#include <stdint.h>
#include <vector>
#include <array>
#include "ByteStream.h"

enum class MessageType : uint16_t
{
    // Client Messages
    Auth            = 1,
    Login,

    CChat,
    
    // Server Messages
    AuthResponse         = 32768,
    LoginResponse,

    SChat,

    SObject,
    Move,
};

const char* to_string(MessageType type);

struct Message
{
    Message(MessageType type) : type(type) {}
    
    const MessageType type;
    
    void pack(ByteStream& buffer) const
    {
        buffer << type;
    }
};

struct LoginMessage : public Message
{
    LoginMessage() : Message(MessageType::Login) {}
    LoginMessage(std::string&& name) : Message(MessageType::Login), name(name) {}
    
    std::string name;
    
    void pack(ByteStream& buffer) const
    {
        Message::pack(buffer);
        buffer << name;
    }

    void unpack(ByteStream& b) {
        b >> name;
    }
};


struct LoginResponseMessage : public Message
{
    LoginResponseMessage() : Message(MessageType::LoginResponse) {}
    
    std::array<float, 3> position{};
};

struct AuthMessage : public Message
{
    AuthMessage() : Message(MessageType::Auth) {}
};

struct AuthResponseMessage : public Message
{
    AuthResponseMessage() : Message(MessageType::AuthResponse) {}
    
    uint64_t sessionId{ 0 };
    
    void pack(ByteStream& buffer) const
    {
        Message::pack(buffer);
        buffer << sessionId;
    }

    void unpack(ByteStream& b) {
        b >> sessionId;
    }
};

struct ClientChatMessage : public Message
{
    ClientChatMessage() : Message(MessageType::CChat) {}
    ClientChatMessage(std::string&& contents) : Message(MessageType::CChat), contents(std::move(contents)) {}
    
    std::string contents;
    
    void pack(ByteStream& buffer) const
    {
        Message::pack(buffer);
        buffer << contents;
    }

    void unpack(ByteStream& b) {
        b >> contents;
    }
};

struct ServerChatMessage : public Message {
    ServerChatMessage() = delete;
    ServerChatMessage(uint64_t guid, std::string&& contents) : Message(MessageType::SChat), _guid(guid), contents(std::move(contents)) {}

    uint64_t _guid{ 0 };
    std::string contents;

    static ServerChatMessage unpack(ByteStream& b) {
        uint64_t guid;
        std::string contents;
        b >> guid;
        b >> contents;
        return ServerChatMessage(guid, std::move(contents));
    }
};

struct ServerObjectMessage : public Message {
    ServerObjectMessage() = delete;
    ServerObjectMessage(uint64_t guid, std::string&& name) : Message(MessageType::SObject), _guid(guid), _name(std::move(name)) {};

    uint64_t _guid{ 0 };
    std::string _name;

    void pack(ByteStream& b) const {
        Message::pack(b);
        b << _guid;
        b << _name;
    }

    static ServerObjectMessage unpack(ByteStream& b) {
        uint64_t guid;
        std::string name;
        b >> guid;
        b >> name;
        return ServerObjectMessage(guid, std::move(name));
    }
};

enum class MoveType : uint8_t
{
    StartMoveUp,
    StartMoveDown,
    StartMoveLeft,
    StartMoveRight,
};

struct MoveMessage : public Message
{
    MoveMessage() = delete;
    MoveMessage(uint64_t guid, MoveType type) : Message(MessageType::Move), _guid(guid), moveType(type) {}
    
    uint64_t _guid;
    MoveType moveType;    
};





