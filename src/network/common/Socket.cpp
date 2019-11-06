//
//  Socket.cpp
//  dirtycommon
//
//  Created by Eugene Sturm on 2/2/19.
//

#include "Socket.hpp"
#include <enet/enet.h>
#include <iostream>
#include "Connection.hpp"

const char* to_string(SocketEventType type)
{
    switch (type) {
        case SocketEventType::PackedSent: return "PackedSent";
        case SocketEventType::PacketReceived: return "PacketReceived";
        case SocketEventType::PeerConnected: return "PeerConnected";
        case SocketEventType::PeerDisconnected: return "PeerDisconnected";
    }
    return "[messageType]";
}

Socket::Socket(SocketEventDelegate&& socketEventDelegate)
: _eventDelegate(std::move(socketEventDelegate))
{
    _host = enet_host_create(NULL /* create a client host */,
                             1 /* only allow 1 outgoing connection */,
                             2 /* allow up 2 channels to be used, 0 and 1 */,
                             0 /* assume any amount of incoming bandwidth */,
                             0 /* assume any amount of incoming bandwidth */);
    
    
    _isServiceThreadRunning = true;
    _serviceThread = std::thread([this]() { this->serviceSocket(); });
}

Socket::Socket(uint64_t listenPort, SocketEventDelegate&& socketEventDelegate)
: _eventDelegate(std::move(socketEventDelegate))
{
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = listenPort;
    _host = enet_host_create(&address /* the address to bind the server host to */,
                             32      /* allow up to 32 clients and/or outgoing connections */,
                             2      /* allow up to 2 channels to be used, 0 and 1 */,
                             0      /* assume any amount of incoming bandwidth */,
                             0      /* assume any amount of outgoing bandwidth */);
    
    _isServiceThreadRunning = true;
    _serviceThread = std::thread([this]() { this->serviceSocket(); });
}

Socket::~Socket()
{
    _isServiceThreadRunning = false;
    _serviceThread.join();
}

std::shared_ptr<Connection> Socket::connect(const std::string& addr, uint16_t port)
{
    ENetAddress address;
    
    enet_address_set_host(&address, addr.c_str());
    address.port = port;
    ENetPeer* peer = enet_host_connect(_host, &address, 2, 0);
    if (peer == nullptr) {
        std::cerr << "No available peers for initiating an ENet connection\n";
        return nullptr;
    }    
    
    auto connection = std::make_shared<Connection>(peer, this);
    _activeConnections.emplace_back(connection);
    connection->setConnectionState(ConnectionState::Connecting);
    
    return connection;
}

void Socket::serviceSocket()
{
    while (_isServiceThreadRunning) {                        
        for (auto& connection : _activeConnections) {
            uint32_t sendCount = connection->flushOutgoingQueue();
            if (sendCount > 0) {
                
                if (_eventDelegate) {
                    _eventDelegate(SocketEventType::PackedSent, connection);
                }
            }
        }
        
        ENetEvent event;
        while (enet_host_service(_host, &event, 50) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    ENetPeer* peer = event.peer;
                    
                    auto it = std::find_if(begin(_activeConnections), end(_activeConnections), [peer](const ConnectionPtr& connection) { return connection->peerInfo() == peer; });
                    
                    if (it != end(_activeConnections)) {
                        ConnectionPtr& connection = *it;
                        if (connection->connectionState() != ConnectionState::Connecting) {
                            // huh?
                        } else {
                            connection->setConnectionState(ConnectionState::Connected);
                        }
                        
                        if (_eventDelegate) {
                            _eventDelegate(SocketEventType::PeerConnected, connection);
                        }
                    } else {
                        auto connection = std::make_shared<Connection>(peer, this);
                        connection->setConnectionState(ConnectionState::Connected);
                        _activeConnections.emplace_back(connection);
                        
                        if (_eventDelegate) {
                            _eventDelegate(SocketEventType::PeerConnected, connection);
                        }
                    }
                    
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT: {
                    ENetPeer* peer = event.peer;
                    auto connectionIt = std::find_if(begin(_activeConnections), end(_activeConnections), [peer](const ConnectionPtr& connection) {
                        return connection->peerInfo() == peer;
                    });
                    
                    if (connectionIt != end(_activeConnections)) {
                        (*connectionIt)->setConnectionState(ConnectionState::Disconnected);
                        _activeConnections.erase(connectionIt);
                        
                        if (_eventDelegate) {
                            _eventDelegate(SocketEventType::PeerDisconnected, *connectionIt);
                        }
                    }
                    
                    
                    break;
                }
                case ENET_EVENT_TYPE_RECEIVE: {
                    ENetPeer* peer = event.peer;
                    
                    auto connectionIt = std::find_if(begin(_activeConnections), end(_activeConnections), [peer](const ConnectionPtr& connection) {
                        return connection->peerInfo() == peer;
                    });
                    
                    if (connectionIt == end(_activeConnections)) {
                        // wat?
                        break;
                    }
                    
                    (*connectionIt)->queueIncoming(Packet(event.packet->data, event.packet->dataLength));
                    enet_packet_destroy(event.packet);
                    
                    if (_eventDelegate) {
                        _eventDelegate(SocketEventType::PacketReceived, *connectionIt);
                    }
                    break;
                }
                case ENET_EVENT_TYPE_NONE:
                {
                    break;
                }
            }
        }        
        
    }
}
