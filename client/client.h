/**
 * Class encodes data for transmission
 */

#include "common/common.h"

#include <iostream>
#include <vector>
#include <string>

using namespace std;

class MccClient {
public:
    MccClient(vector<uint8_t> & tx_buffer) : tx_buffer_(tx_buffer) {}
    /**
     * Encodes a request to register a user
     * @param username - string representing the username to register
     * @return # of bytes encoded
     */
    size_t encodeRequestRegister(const string& username);

    /**
     * Encodes a request to get online users
     * @param id - user ID of current user
     * @return # of bytes encoded
     */
    size_t encodeRequestUsers(user_id_t id);

    /**
     * Encodes a request to send a message to a user
     * @param src_id - source user ID identifying user sending the message
     * @param target_id - ID identifying user whom the message is intended for
     * @param msg - the message to send
     * @return # of bytes encoded
     */
    size_t encodeRequestSend(user_id_t src_id, user_id_t target_id, const string&  msg);
    
    /**
     * Encodes a request to receive packets from server
     * @param id - user ID for whom messages are to be received
     * @return # of bytes encoded
     */
    size_t encodeRequestRecv(user_id_t id);

private:
    vector<uint8_t>& tx_buffer_; // ref. to buffer where encoded packets are stored
};