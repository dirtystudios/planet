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
#include "Packet.h"
#include "Connection.h"
#include <thread>
#include <map>
#include <unordered_map>
#include <mutex>
#include <atomic>

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

struct _ENetHost;

class Socket
{
private:
    _ENetHost* _host { nullptr };
    SocketEventDelegate _eventDelegate;
    std::thread _serviceThread;
    std::atomic<bool> _isServiceThreadRunning { false };
    
    std::vector<ConnectionPtr> _activeConnections;
    
    std::unordered_map<ConnectionPtr, std::mutex> _connectionMutexes;
    std::unordered_map<ConnectionPtr, std::vector<Packet>> _incomingQueue;
    std::unordered_map<ConnectionPtr, std::vector<Packet>> _outgoingQueues;
public:
    Socket(SocketEventDelegate&& socketEventDelegate = SocketEventDelegate());
    Socket(uint16_t listenPort, SocketEventDelegate&& socketEventDelegate = SocketEventDelegate());
    ~Socket();
    
    std::shared_ptr<Connection> connect(const std::string& addr, uint16_t port, ConnectionStateDelegate&& d = ConnectionStateDelegate());    
private:
    void serviceSocket();
};





