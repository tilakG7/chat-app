#include "server.h"
#include "database.h" // TODO: Remove, this is only needed for testing

#include <iostream>
#include <iomanip> // For std::hex, std::setw, and std::setfill
#include <cstdint> // For uint8_t

using namespace std;

size_t MccServer::parseRequestRegister(uint8_t *data, length_t payload_len) {
    // read username from request and register user
    string username(reinterpret_cast<char *>(data), payload_len);
    user_id_t id = db_.registerUser(username); // unique ID assigned to user
    
    // construct response
    uint8_t *dest = tx_buffer_.data();
    uint8_t resp_value = 0; // success, user registeration always returns valid user ID
    const length_t payload_size =  sizeof(resp_value) + sizeof(user_id_t);
    Header resp_hdr{PacketType::kRespRegister, payload_size};
    
    // copy header
    memcpy(dest, &resp_hdr, sizeof(resp_hdr));
    dest += sizeof(resp_hdr);
    // copy response value
    *dest = resp_value;
    dest++;
    // copy user ID
    memcpy(dest, &id, sizeof(user_id_t));
    dest += sizeof(user_id_t);

    return sizeof(Header) + payload_size;
}

size_t MccServer::parseRequestUsers(uint8_t *data, length_t payload_len) {
    assert(payload_len == sizeof(user_id_t) && "Users request should have a size of 4");

    user_id_t id = *reinterpret_cast<user_id_t*>(data);

    // construct response
    Header resp_hdr;
    resp_hdr.type = PacketType::kRespUsers;
    uint8_t *dest = &tx_buffer_[0];
    
    // Return a negative response if user ID not in server storage
    if(!db_.userExists(id)) {
        uint8_t resp_value = 1; // error, user ID not in database
        resp_hdr.len = sizeof(resp_value);
        memcpy(dest, &resp_hdr, sizeof(resp_hdr));
        dest += sizeof(resp_hdr);
        *dest = resp_value;
        dest++;
        return sizeof(Header) + resp_hdr.len;
    }

    uint8_t resp_value = 0;
    // number of online users, except for current user
    user_id_t num_users = db_.getNumUsers() - 1; 

    if(num_users == 0) {
        resp_hdr.len = sizeof(resp_value) + sizeof(user_id_t);
        // copy header
        memcpy(dest, &resp_hdr, sizeof(resp_hdr));
        dest += sizeof(resp_hdr);
        // copy response value
        *dest = resp_value;
        dest += sizeof(resp_value);
        // copy user ID
        *reinterpret_cast<user_id_t*>(dest) = num_users;
        dest += sizeof(user_id_t);

        return sizeof(Header) + resp_hdr.len;
    }

    length_t curr_payload_len = sizeof(Header);
    dest += sizeof(Header);
    // copy response value
    *dest = resp_value;
    dest += sizeof(resp_value);
    curr_payload_len += sizeof(resp_value);
    // copy number of users
    *reinterpret_cast<user_id_t*>(dest) = num_users;
    dest += sizeof(user_id_t);
    curr_payload_len += sizeof(user_id_t);

    for(auto &it : db_) {
        // current user should only know which OTHER users are online
        if(it.first == id) {
            continue;
        }
        
        // copy user ID
        *reinterpret_cast<user_id_t*>(dest) = it.first;
        dest += sizeof(user_id_t);
        // copy username size
        *reinterpret_cast<uint32_t*>(dest) = it.second.username.size();
        dest += sizeof(uint32_t);
        // copy username
        memcpy(dest, it.second.username.data(), it.second.username.size());
        dest += it.second.username.size();
        
        curr_payload_len += sizeof(user_id_t) + sizeof(uint32_t) + it.second.username.size();
    }
    resp_hdr.len = curr_payload_len;

    // set header
    memcpy(&tx_buffer_[0], &resp_hdr, sizeof(Header));

    return sizeof(Header) + resp_hdr.len;
}

size_t MccServer::parseRequestSend(uint8_t *data, length_t payload_len) {
    assert(payload_len > (sizeof(user_id_t) * 2) && "Payload must hold 2 user ids and a message");
    // payload contains the message, and source and target user IDs 
    const size_t msg_len = payload_len - (sizeof(user_id_t) * 2); 

    // get source ID
    user_id_t source_id = *reinterpret_cast<user_id_t*>(data);
    data += sizeof(user_id_t);
    // get target ID
    user_id_t target_id = *reinterpret_cast<user_id_t*>(data);
    data += sizeof(user_id_t);
    // get message
    string msg(reinterpret_cast<char*>(data), msg_len);

    cout << "Received message : " << msg << endl;
    cout << "From: " << source_id << ", for: " << target_id << endl;

    // construct response
    uint8_t resp_value = 0;
    Header h{PacketType::kRespSend, sizeof(resp_value)};
    if(!db_.userExists(source_id)) {
        resp_value = 1;
    } else if(!db_.userExists(target_id)) {
        resp_value = 2;
    } else {
        // store message
        db_.queueMsg(target_id, {source_id, msg});
    }
    
    uint8_t *dest = &tx_buffer_[0];
    // copy the Header
    memcpy(dest, &h, sizeof(Header));
    dest += sizeof(Header);
    memcpy(dest, &resp_value, sizeof(resp_value));
    dest += sizeof(resp_value);

    return sizeof(Header) + sizeof(uint8_t);
}

size_t MccServer::parseRequestRecv(uint8_t *data, length_t payload_len) {
    assert(payload_len == sizeof(user_id_t) && 
            "Request to receive messages has incorrect payload length");

    user_id_t id = *reinterpret_cast<user_id_t*>(data);

    // construct response
    Header resp_hdr;
    resp_hdr.type = PacketType::kRespRecv;
    uint8_t *dest = &tx_buffer_[0];
    
    // Return a negative response if user ID not in server storage
    if(!db_.userExists(id)) {
        uint8_t resp_value = 1; // error, user ID not in database
        resp_hdr.len = sizeof(resp_value);
        memcpy(dest, &resp_hdr, sizeof(resp_hdr));
        dest += sizeof(resp_hdr);
        *dest = resp_value;
        dest++;
        return sizeof(Header) + resp_hdr.len;
    }

    length_t resp_pl = 0;
    dest += sizeof(Header);
    resp_pl += sizeof(Header);

    // copy the response value
    uint8_t resp_value = 0; // success, user ID found in database
    memcpy(dest, &resp_value, sizeof(resp_value));
    dest += sizeof(resp_value);
    resp_pl += sizeof(resp_value);
    // copy the number of messages
    uint32_t num_messages = db_[id].msg_q.size();
    memcpy(dest, &num_messages, sizeof(num_messages));
    dest += sizeof(num_messages);
    resp_pl += sizeof(num_messages);

    for(Msg &m : db_[id].msg_q) {
        // copy the sender ID
        memcpy(dest, &m.sender_id, sizeof(m.sender_id));
        dest += sizeof(m.sender_id);
        resp_pl += sizeof(m.sender_id);
        // copy the length of message
        uint32_t msg_len = m.msg.size();
        memcpy(dest, &msg_len, sizeof(msg_len));
        dest += sizeof(msg_len);
        resp_pl += sizeof(msg_len);
        // copy the message
        memcpy(dest, m.msg.data(), msg_len);
        dest += msg_len;
        resp_pl += msg_len;
    }

    // populate the header
    resp_hdr.len = resp_pl;
    memcpy(&tx_buffer_[0], &resp_hdr, sizeof(resp_hdr));

    return sizeof(Header) + resp_pl;
}

size_t MccServer::parse(uint8_t *data, size_t size) {
    Header h = *reinterpret_cast<Header*>(data);
    assert(size >= (sizeof(Header) + kMinPayloadLen) && "Invalid packet length");
    assert(h.len == (size - sizeof(Header)) && "Invalid payload length");

    switch(h.type) {
        case PacketType::kRequestRegister:
            return parseRequestRegister(data + sizeof(Header), h.len);
        break;
        case PacketType::kRequestUsers:
            return parseRequestUsers(data + sizeof(Header), h.len);
        break;
        case PacketType::kRequestSend:
            return parseRequestSend(data + sizeof(Header), h.len);
        break;
        case PacketType::kRequestRecv:
            return parseRequestRecv(data + sizeof(Header), h.len);
        break;
        case PacketType::kRespRegister: // intentional fall-through
        case PacketType::kRespUsers:    // intentional fall-through
        case PacketType::kRespSend:     // intentional fall-through
        case PacketType::kRespRecv:
            cerr << "ERR: Server received packet type meant for client" << endl;
        default:
            cerr << "ERR: Server received unknown packet type "<< endl;

    }
    return 0;
}

//c++ -std=c++20 server_app.cpp database.cpp server.cpp -I . -I ../ -o b && ./b