/**
 * Implements the Multiple Computer Communication protocol - server side
 */
// TODO: Try using protobufs instead?
#pragma once 

#include <iostream>

#include "database.h"

#include "common/common.h"

using namespace std;

constexpr size_t kMinPayloadLen = 1; // for server side

class MccServer {
public:
    MccServer(Database& db, vector<uint8_t> &tx_buffer) 
        : db_(db), tx_buffer_(tx_buffer) {}

    // /**
    //  * For now, prints the response for verification
    //  * TODO: actually send the response over the network
    //  * @param num_bytes - number of bytes to send 
    //  */
    // void sendResp(size_t num_bytes);

    /**
     * Parses the register request
     * @param data        - pointer to payload of register request
     * @param payload_len - length of payload
     * @return size of response in number of bytes (stored in tx_buffer_)
     */
    size_t parseRequestRegister(uint8_t *data, length_t payload_len);

    /**
     * Parses the get online users request
     * @param data         - pointer to payload of request
     * @param payload_len   
     * @return size of response in number of bytes (stored in tx_buffer_)
     */
    size_t parseRequestUsers(uint8_t *data, length_t payload_len);

    /**
     * If user A is trying to message user B, this function stores user A's 
     * message in the database for retrieval when user B checks for new messages
     * @param data - pointer to payload of request
     * @param payload_len 
     * @return size of response in number of bytes (stored in tx_buffer_)
     */
    size_t parseRequestSend(uint8_t *data, length_t payload_len);

    /**
     * Parses a request from the user to get all relevant messages
     * @param data - pointer to payload of the request
     * @param payload_len
     * @return size of response in number of bytes (stored in tx_buffer_)
     */
    size_t parseRequestRecv(uint8_t *data, length_t payload_len);

    /**
     * Parses 1 request from the client and sends a response back
     * @param data - pointer to data buffer
     * @param size - number of bytes in buffer pointed to by data
     * @returns size of the response in number of bytes (stored in tx_buffer_)
     */
    size_t parse(uint8_t *data, size_t size);

private:
    Database& db_;   // reference to the database instance
    vector<uint8_t>& tx_buffer_; // buffer for data to transmit to client
};