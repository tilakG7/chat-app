/**
 * Implements the Chat Communication Protocol - server side
 */
#include "server.h"

#include <iostream>

using namespace std;

void CCPServer::parseRequestRegister(uint8_t *data, length_t payload_len) {
    string username(reinterpret_cast<char *>(data), payload_len);
    user_id_t id = storage_.registerUser(username);
    
    // construct response
    uint8_t resp_value = 1; // success, user registeration always returns valid user ID
    length_t kPayloadSize =  sizeof(resp_value) + sizeof(user_id_t);
    Header resp_hdr{PacketType::kRespRegister, kPayloadSize};
    memcpy(&tx_buffer_[0], &resp_hdr, sizeof(resp_hdr));
    tx_buffer_[sizeof(resp_hdr)] = resp_value;
    memcpy(&tx_buffer_[sizeof(resp_hdr) + sizeof(resp_value)], &id, sizeof(user_id_t));
    // send buffered data in tx_ id back to the user
    // size of data = sizeof(Header) + kPayloadSize
}

void CCPServer::parseRequestUsers(uint8_t *data, length_t payload_len) {
    
}

void CCPServer::parse(uint8_t *data, size_t size) {
    Header h = *reinterpret_cast<Header*>(data);
    assert(size >= (sizeof(Header) + kMinPayloadLen) && "Invalid packet length");
    assert(h.len == (size - sizeof(Header)) && "Invalid payload length");

    switch(h.type) {
        case PacketType::kRequestRegister:
            parseRequestRegister(data + sizeof(Header), h.len);
        break;
        case PacketType::kRequestUsers:
            parseRequestUsers(data + sizeof(Header), h.len);
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
