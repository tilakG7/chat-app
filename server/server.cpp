#include "server.h"
#include "database.h" // TODO: Remove, this is only needed for testing

#include <iostream>
#include <iomanip> // For std::hex, std::setw, and std::setfill
#include <cstdint> // For uint8_t

using namespace std;

void MccServer::parseRequestRegister(uint8_t *data, length_t payload_len) {
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
    
    sendResp(sizeof(Header) + payload_size);
}

void MccServer::parseRequestUsers(uint8_t *data, length_t payload_len) {
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
        sendResp(sizeof(Header) + resp_hdr.len);
        return;
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

        sendResp(sizeof(Header) + resp_hdr.len);
        return;
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

    // send the reponse
    sendResp(resp_hdr.len + sizeof(Header));
}

void MccServer::parseRequestSend(uint8_t *data, length_t payload_len) {
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

    sendResp(sizeof(Header) + sizeof(uint8_t));

}

void MccServer::parse(uint8_t *data, size_t size) {
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
            parseRequestSend(data + sizeof(Header), h.len);
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

void handleRespRegister(uint8_t *payload, length_t len) {
    cout << "Received kRespRegister" << endl;
    cout << "Payload length: " << len << endl;
    for(length_t i=0; i < len; i++) {
        cout <<  std::hex       // Switch to hexadecimal format
             << std::setw(2)    // Minimum width of 2 characters
             << std::setfill('0') // Fill with '0' if width is less
             << static_cast<int>(*payload) // Cast to int to avoid interpreting as char
             << std::endl;
        payload++;
    }
}

void handleRespUsers(uint8_t *payload, length_t len) {
    cout << "Received kRespUsers" << endl;
    cout << "Payload length: " << len << endl;
    uint8_t resp_value = *payload;
    payload++;

    if(resp_value != 0) {
        cerr << "ERR: Users response failed" << endl;
        return;
    }
    // get the number of users
    user_id_t num_users = *reinterpret_cast<user_id_t*>(payload);
    payload += sizeof(user_id_t);

    for(user_id_t i=0; i < num_users; i++) {
        // get user ID
        const user_id_t curr_user_id = *reinterpret_cast<user_id_t*>(payload);
        payload += sizeof(user_id_t);
        // get username size
        const uint32_t username_size = *reinterpret_cast<uint32_t*>(payload);
        payload += sizeof(uint32_t);
        // get username
        string username(reinterpret_cast<char*>(payload), username_size);
        payload += username_size;

        cout << "user ID : " << curr_user_id << endl;
        cout << "username: " << username << endl;
    }
}

void handleRespSend(uint8_t *payload, length_t len) {
    cout << "Received kRespSend" << endl;
    cout << "Payload length: " << len << endl;
    for(length_t i=0; i < len; i++) {
        cout << static_cast<int>(*payload) << endl;
    }
}

void handleRespRecv(uint8_t *payload, length_t len) {
    cout << "Received kRespRecv" << endl;
    cout << "Payload length: " << len << endl;
    for(length_t i=0; i < len; i++) {
        cout << static_cast<int>(*payload) << endl;
    }
}

void MccServer::sendResp(size_t num_bytes) {
    uint8_t *data = &tx_buffer_[0];
    Header h = *reinterpret_cast<Header*>(data);
    data += sizeof(Header);
    
    switch(h.type) {
        case PacketType::kRespRegister: 
            handleRespRegister(data, h.len);
        break;
        case PacketType::kRespUsers:
            handleRespUsers(data, h.len);
        break;
        case PacketType::kRespSend:  
            handleRespSend(data, h.len);   
        break;
        case PacketType::kRespRecv:
            handleRespRecv(data, h.len);
        break;
        case PacketType::kRequestRegister: // intentional fall-through
        case PacketType::kRequestUsers: // intentional fall-through
        case PacketType::kRequestSend: // intentional fall-through
        case PacketType::kRequestRecv: // intentional fall-through
        default:
            cerr << "ERR: Received invalid packet from server" << endl;
        break;
    }

}

int main() {
    vector<uint8_t> tx_buffer(100, 0);
    MccServer server(Database::getInstance(), tx_buffer);

    vector<uint8_t> req_register{0x00, 6, 0, 0, 0, 'e', 'v', 'e', 'l', 'y', 'n'}; 
    server.parse(&req_register[0], req_register.size());

    vector<uint8_t> req_register2{0x00, 6, 0, 0, 0, 'e', 'v', 'e', 'r', 'e', 't'}; 
    server.parse(&req_register2[0], req_register2.size());\

    vector<uint8_t> req_register3{0x00, 4, 0, 0, 0, 'j', 'a', 'c', 'k'}; 
    server.parse(&req_register3[0], req_register3.size());

    vector<uint8_t> req_users{0x01, 4, 0, 0, 0, 2, 0, 0, 0};
    server.parse(&req_users[0], req_users.size());
}