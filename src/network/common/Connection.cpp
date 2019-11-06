//
//  Connection.cpp
//  dirtycommon
//
//  Created by Eugene Sturm on 2/3/19.
//

#include "Connection.hpp"
#include "Socket.hpp"
#include <enet/enet.h>

Connection::Connection(ENetPeer* peer, Socket* socket)
: _peer(peer)
, _socket(socket)
{}

void Connection::waitForConnectionState(ConnectionState state)
{
    std::unique_lock<std::mutex> lck(_connectionStateMutex);
    while (_state != state) {
        _connectionStateCondition.wait(lck);
    }
}

void Connection::setConnectionState(ConnectionState state)
{
    std::lock_guard<std::mutex> lock(_connectionStateMutex);
    _state = state;
    _connectionStateCondition.notify_one();
}

Connection::~Connection()
{
    disconnect();
    enet_peer_reset(_peer);
}

void Connection::disconnect()
{
    enet_peer_disconnect(_peer, 0);
}

void Connection::queueIncoming(Packet&& packet)
{
    std::lock_guard<std::mutex> lock(_recvQueueMutex);
    _incomingQueue.emplace_back(packet);
    _incomingCondition.notify_one();
}

void Connection::queueOutgoing(Packet&& packet)
{
    std::lock_guard<std::mutex> lock(_sendQueueMutex);
    _sendQueue.emplace_back(packet);
}

Packet Connection::waitForIncoming()
{
     std::unique_lock<std::mutex> lck(_recvQueueMutex);
    _incomingCondition.wait(lck);
    
    Packet result = std::move(_incomingQueue.front());
    _incomingQueue.pop_front();
    return result;
}

void Connection::drainIncomingQueue(std::vector<Packet>* output)
{
    if (output == nullptr) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(_recvQueueMutex);    
    std::move(begin(_incomingQueue), end(_incomingQueue), std::insert_iterator(*output, output->end()));
    _incomingQueue.clear();
}

std::optional<Packet> Connection::dequeueIncoming()
{
    std::lock_guard<std::mutex> lock(_recvQueueMutex);
    if (_incomingQueue.empty()) {
        return {};
    }
    
    auto result = std::make_optional(std::move(_incomingQueue.front()));
    _incomingQueue.pop_front();
    return result;
}

uint32_t Connection::flushOutgoingQueue()
{
    uint32_t count = 0;
    std::lock_guard<std::mutex> lock(_sendQueueMutex);
    count = _sendQueue.size();
    for (const Packet& packet : _sendQueue) {
        ENetPacket* enetPacket = enet_packet_create(packet.data(), packet.size(), ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(_peer, 0, enetPacket);
    }
    _sendQueue.clear();
    
    return count;
}
