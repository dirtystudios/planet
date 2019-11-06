//
//  Socket.hpp
//  dirtycommon
//
//  Created by Eugene Sturm on 2/2/19.
//

#pragma once

#include <string>
#include <stdint.h>
#include <functional>
#include "Packet.hpp"
#include <thread>
#include <map>
#include <unordered_map>
#include <mutex>

class Connection;
using ConnectionPtr = std::shared_ptr<Connection>;

enum class SocketEventType : uint8_t
{
    PacketReceived,
    PackedSent,
    PeerConnected,
    PeerDisconnected,
};

const char* to_string(SocketEventType type);

using SocketEventDelegate = std::function<void(SocketEventType, ConnectionPtr&)>;

class _ENetHost;

class Socket
{
private:
    _ENetHost* _host { nullptr };
    SocketEventDelegate _eventDelegate;
    std::thread _serviceThread;
    bool _isServiceThreadRunning { false };
    
    std::vector<ConnectionPtr> _activeConnections;
    
    std::unordered_map<ConnectionPtr, std::mutex> _connectionMutexes;
    std::unordered_map<ConnectionPtr, std::vector<Packet>> _incomingQueue;
    std::unordered_map<ConnectionPtr, std::vector<Packet>> _outgoingQueues;
public:
    Socket(SocketEventDelegate&& socketEventDelegate = SocketEventDelegate());
    Socket(uint64_t listenPort, SocketEventDelegate&& socketEventDelegate = SocketEventDelegate());
    ~Socket();
    
    std::shared_ptr<Connection> connect(const std::string& addr, uint16_t port);    
private:
    void serviceSocket();
};





