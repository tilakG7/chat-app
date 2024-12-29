#include "client.h"
#include "common/common.h"
#include <iostream>


size_t MccClient::encodeRequestRegister(const string& username){
    Header h{PacketType::kRequestRegister, static_cast<length_t>(username.size())};
    uint8_t *pDest = &tx_buffer_[0];

    // copy header
    memcpy(pDest, &h, sizeof(Header));
    pDest += sizeof(Header);
    // copy username
    memcpy(pDest, username.data(), username.size());
    pDest += username.size();

    return sizeof(Header) + username.size();
}

size_t MccClient::encodeRequestUsers(user_id_t id){
    Header h{PacketType::kRequestUsers, kReqUsersPayloadLen};
    uint8_t *pDest = &tx_buffer_[0];

    // copy header
    memcpy(pDest, &h, sizeof(Header));
    pDest += sizeof(Header);
    // copy user ID
    memcpy(pDest, &id, sizeof(id));
    pDest += sizeof(id);

    return sizeof(Header) + kReqUsersPayloadLen;
}

size_t MccClient::encodeRequestSend(user_id_t src_id, user_id_t target_id, const string&  msg){
    Header h{PacketType::kRequestSend, static_cast<length_t>(kReqSendMinPayloadLen + msg.size())};
    uint8_t *pDest = &tx_buffer_[0];

    // copy header
    memcpy(pDest, &h, sizeof(Header));
    pDest += sizeof(Header);
    // copy user iD
    memcpy(pDest, &src_id, sizeof(src_id));
    pDest += sizeof(src_id);
    // copy target ID
    memcpy(pDest, &target_id, sizeof(target_id));
    pDest += sizeof(target_id);
    // copy message
    memcpy(pDest, msg.data(), msg.size());
    pDest += msg.size();

    return sizeof(Header) + kReqSendMinPayloadLen + msg.size();
}

size_t MccClient::encodeRequestRecv(user_id_t id){
    Header h{PacketType::kRequestRecv, kReqRecvPayloadLen};
    uint8_t *pDest = &tx_buffer_[0];

    // copy header
    memcpy(pDest, &h, sizeof(Header));
    pDest += sizeof(Header);
    // copy user ID
    memcpy(pDest, &id, sizeof(id));
    pDest += sizeof(id);
    
    return sizeof(Header) + kReqRecvPayloadLen;
}

bool MccClient::validateRespHeader(const PacketType& expected_type) {
    Header h = *reinterpret_cast<Header*>(&rx_buffer_[0]);
    return h.type == expected_type;
}

bool MccClient::handleRespRegister(user_id_t &id) {
    if(!validateRespHeader(PacketType::kRespRegister)) {
        cerr << "ERR: Unexpected response type, expected kRespRegister" << endl;
        return false;
    }

    uint8_t *data = &rx_buffer_[0];
    data += sizeof(Header);

    // read response value
    uint8_t resp_value = *data;
    data++;

    if(resp_value != 0) {
        cerr << "ERR: Unexpected response value in handleRespRegister" << endl;
        return false;
    }

    // read the user ID
    id = *reinterpret_cast<user_id_t*>(data);
    data += sizeof(user_id_t);

    return true;
}


bool MccClient::handleRespUsers(unordered_map<string, user_id_t>& name_to_id) {
    if(!validateRespHeader(PacketType::kRespUsers)) { 
        cerr << "ERR: Unexpected response type, expected kRespUsers" << endl;
        return false; 
    }

    uint8_t *payload = &rx_buffer_[sizeof(Header)];
    uint8_t resp_value = *payload;
    payload++;

    if(resp_value != 0) {
        cerr << "ERR: Users response failed" << endl;
        return false;
    }
    
    // get the number of users
    user_id_t num_users = *reinterpret_cast<user_id_t*>(payload);
    payload += sizeof(user_id_t);

    name_to_id.clear(); // clear existing cache
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

        name_to_id[username] = curr_user_id;
    }
    return true;
}

bool MccClient::handleRespSend(SendStatus &stat) {
    if(!validateRespHeader(PacketType::kRespSend)) {
        cerr << "ERR: Unexpected response type, expected kResSend" << endl;
        return false;
    }

    // get the response value from the server
    uint8_t *payload = &rx_buffer_[sizeof(Header)];
    stat = static_cast<SendStatus>(*payload);
    payload++;

    return true;
}

bool MccClient::handleRespRecv(vector<Msg> &msgs) {
    if(!validateRespHeader(PacketType::kRespRecv)) {
        cerr << "ERR: Unexpected response type, expected kRespRecv" << endl;
        return false;
    }

    // get the response from the server
    uint8_t *payload =  &rx_buffer_[sizeof(Header)];
    
    uint8_t resp_val = *payload;
    payload += sizeof(resp_val);
    if(resp_val > 0) {
        cerr << "ERR: server returned negative response in response receive" << endl;
    }

    uint32_t num_messages = *reinterpret_cast<uint32_t*>(payload);
    payload += sizeof(num_messages);

    for(length_t i=0; i < num_messages; i++) {
        user_id_t id = *reinterpret_cast<user_id_t*>(payload);
        payload += sizeof(user_id_t);
        length_t msg_len = *reinterpret_cast<length_t*>(payload);
        payload += sizeof(length_t);
        string msg(reinterpret_cast<char*>(payload), msg_len);
        payload += msg_len;

        msgs.push_back({id, msg});
    }
    return true;
}

