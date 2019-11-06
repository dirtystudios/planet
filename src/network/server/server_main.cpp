//
//  main.cpp
//  dirtyjam
//
//  Created by Eugene Sturm on 1/25/19.
//

#include "Socket.hpp"
#include "Connection.hpp"
#include <iostream>
#include "World.hpp"
#include "Session.hpp"
#include "Database.hpp"

int main(int argc, char** argv)
{
    std::shared_ptr<Database> db(new Database());
    
    std::mutex enteringConnectionsMutex;
    std::vector<ConnectionPtr> enteringConnections;
    
    std::mutex exitingConnectionsMutex;
    std::vector<ConnectionPtr> exitingConnections;
    
    std::vector<ConnectionPtr> pendingConnections;
    std::vector<ConnectionPtr> activeConnections;
    
    auto eventDelegate = [&](SocketEventType type, ConnectionPtr& connection)
    {
        switch (type) {
            case SocketEventType::PeerConnected: {
                std::lock_guard<std::mutex> lock(enteringConnectionsMutex);
                enteringConnections.emplace_back(connection);                
                break;
            }
            case SocketEventType::PeerDisconnected: {
                std::lock_guard<std::mutex> lock(exitingConnectionsMutex);
                exitingConnections.emplace_back(connection);
                break;
            }
            default: {
                break;
            }
        }
    };
    
    Socket socket(44951, eventDelegate);
    World world(db);
    
    while (true) {
        {
            std::lock_guard<std::mutex> lock(enteringConnectionsMutex);
            pendingConnections.insert(end(pendingConnections), begin(enteringConnections), end(enteringConnections));
            enteringConnections.clear();
        }
        
        Packet packet;
        for (auto it = begin(pendingConnections); it != end(pendingConnections);) {
            auto& connection = *it;
            
            bool didAuth = false;
            std::optional<Packet> dequeueResult;
            while ( (dequeueResult = connection->dequeueIncoming()).has_value() ) {
                Packet& packet = dequeueResult.value();
                
                const MessageType type = packet.messageType();
                if (type == MessageType::Auth) {
                    connection->queueOutgoing(Packet(AuthResponseMessage()));
                    didAuth = true;
                    break;
                } else {
                    std::cout << "dropping " << to_string(type) << std::endl;
                    // drop others
                }
            }
            
            if (didAuth) {
                it = pendingConnections.erase(it);
                activeConnections.emplace_back(connection);
                world.addSession(std::make_shared<Session>(connection));
            } else {
                ++it;
            }
        }
        
        world.update();
                
    }
}

