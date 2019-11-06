//
//  Connection.hpp
//  dirtycommon
//
//  Created by Eugene Sturm on 2/3/19.
//

#pragma once

#include <memory>
#include <stdint.h>
#include <mutex>
#include <queue>
#include <condition_variable>
#include "Packet.hpp"
#include <optional>
#include <array>

class _ENetPeer;
class Socket;

enum class ConnectionState : uint8_t
{
    Disconnected,
    Connecting,
    Connected,
};

class Connection
{
private:
    _ENetPeer* _peer { nullptr };
    Socket* _socket { nullptr };
    ConnectionState _state { ConnectionState::Disconnected };
    
    std::mutex _sendQueueMutex;
    std::vector<Packet> _sendQueue;
    
    std::mutex _recvQueueMutex;    
    std::deque<Packet> _incomingQueue;
    
    std::condition_variable _incomingCondition;
    
    std::mutex _connectionStateMutex;
    std::condition_variable _connectionStateCondition;
public:
    Connection(_ENetPeer* peer, Socket* socket);
    ~Connection();
    
    void disconnect();
    
    void queueIncoming(Packet&& packet);
    void queueOutgoing(Packet&& packet);
    
    std::optional<Packet> dequeueIncoming();
    Packet waitForIncoming();
    
    void drainIncomingQueue(std::vector<Packet>* output);
    uint32_t flushOutgoingQueue();
    
    _ENetPeer* peerInfo() const { return _peer; }
    ConnectionState connectionState() const { return _state; }
    
    void waitForConnectionState(ConnectionState state);
protected:
    void setConnectionState(ConnectionState state);
    
    
    
    friend class Socket;
};

using ConnectionPtr = std::shared_ptr<Connection>;
