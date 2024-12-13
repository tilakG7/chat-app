/**
 * Implements the Multiple Computer Communication protocol - server side
 */
// TODO: Try using protobufs instead?
#pragma once 

#include <iostream>

#include "database.h"

using namespace std;
using length_t = uint32_t;
enum class PacketType : uint8_t {
    kRequestRegister = 0,
    kRequestUsers = 1,
    kRequestSend = 2,
    kRequestRecv = 3,
    kRespRegister = 0xF0,
    kRespUsers = 0xF1,
    kRespSend = 0xF2,
    kRespRecv = 0xF3
};

struct __attribute__((packed)) Header {
    PacketType type;
    length_t len;
};

constexpr size_t kMinPayloadLen = 1; // for server side

class MccServer {
public:
    MccServer(Database& db, vector<uint8_t> &tx_buffer) 
        : db_(db), tx_buffer_(tx_buffer) {}

    /**
     * For now, prints the response for verification
     * TODO: actually send the response over the network
     * @param num_bytes - number of bytes to send 
     */
    void sendResp(size_t num_bytes);

    /**
     * Parses the register request
     * @param data        - pointer to payload of register request
     * @param payload_len - length of payload
     */
    void parseRequestRegister(uint8_t *data, length_t payload_len);

    /**
     * Parses the get online users request
     * @param data         - pointer to payload of request
     * @param payload_len   
     */
    void parseRequestUsers(uint8_t *data, length_t payload_len);

    /**
     * If user A is trying to message user B, this function stores user A's 
     * message in the database for retrieval when user B checks for new messages
     * @param data - pointer to payload of request
     * @param payload_len 
     */
    void parseRequestSend(uint8_t *data, length_t payload_len);

    /**
     * Parses a request from the user to get all relevant messages
     * @param data - pointer to payload of the request
     * @param payload_len
     */
    void parseRequestRecv(uint8_t *data, length_t payload_len);

    /**
     * Parses 1 request from the client and sends a response back
     * @param data - pointer to data buffer
     * @param size - number of bytes in buffer pointed to by data
     */
    void parse(uint8_t *data, size_t size);

private:
    Database& db_;   // reference to the database instance
    vector<uint8_t>& tx_buffer_; // buffer for data to transmit to client
};