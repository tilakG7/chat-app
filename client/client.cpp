#include "client.h"

#include <iostream>


size_t MccClient::encodeRequestRegister(const string& username){
    Header h{PacketType::kRequestRegister, username.size()};
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
    Header h{PacketType::kRequestSend, kReqSendMinPayloadLen + msg.size()};
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