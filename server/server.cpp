/**
 * Implements the server class
 */
#include "server.h"

#include <iostream>

using namespace std;

void Server::parseRequestRegister(uint8_t *data, length_t payload_len) {
    string username(reinterpret_cast<char *>(data), payload_len);
    user_id_t id = storage_.registerUser();
    // return id back to the user
}

void Server::parse(uint8_t *data, size_t size) {
    Header h = *reinterpret_cast<Header*>(data);
    assert(size >= (sizeof(Header) + kMinPayloadLen) && "Invalid packet length");
    assert(h.len == (size - sizeof(Header)) && "Invalid payload length");

    switch(h.type) {
        case PacketType::kRequestRegister:

        break;
        case PacketType::kRequestUsers:
        break;
        case PacketType::kRequestSend:
        break;
        case PacketType::kRequestRecv:
        break;
        case PacketType::kRespRegister: // intentional fall-through
        case PacketType::kRespUsers:    // intentional fall-through
        case PacketType::kRespSend:     // intentional fall-through
        case PacketType::kRespRecv:
            cerr << "ERR: Server received packet type meant for client" << endl;
        default:
            cerr << "ERR: Server received unknown packet type "<< endl;

    }

}
