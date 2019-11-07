#include "ClientSession.h"
#include "Log.h"
#include "Connection.h"
#include "DGAssert.h"

ClientSession::ClientSession(const ConnectionPtr& connection)
:_connection(connection) {

}

void ClientSession::processIncoming() {
    std::vector<Packet> packets;
    _connection->drainIncomingQueue(&packets);

    for (const Packet& packet : packets) {
        const MessageType type = packet.read<MessageType>();

        LOG_D("ClientSession Recv: %s", to_string(type));

        switch (type) {
        case MessageType::AuthResponse: {
            _status = SessionStatus::Authed;
            break;
        }
        case MessageType::LoginResponse: {
            _status = SessionStatus::LoggedIn;
            break;
        }
        default: {
            break;
        }
        }
    }
}

void ClientSession::registerHandler(MessageType type, ClientPacketHandler&& handler) {
    dg_assert_fail_nm();
}

void ClientSession::registerHandler(SessionStatus type, ClientSessionHandler&& handler) {
    dg_assert_fail_nm();
}