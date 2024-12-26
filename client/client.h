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
    MccClient(vector<uint8_t> & tx_buffer, vector<uint8_t> &rx_buffer) : 
        tx_buffer_(tx_buffer), rx_buffer_(rx_buffer) {}
        
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

    /**
     * Handles the response to the register user request
     * @param[out] id - holds the id of the newly registered user
     * @returns true if the response was handled successfully
     */
    bool handleRespRegister(user_id_t &id);

    /**
     * Handles the response to the online users request
     * @param[out] name_to_id - updated to hold a valid mapping of names to user 
     *  IDs if function returns true
     * @returns true if the response was handled successfully
     */
    bool handleRespUsers(unordered_map<string, user_id_t>& name_to_id);

    /**
     * Handles the response to the request to send data to a user
     * @param[out] stat - holds the response value from the server
     * @returns true if the response was successfully handled
     */
    bool handleRespSend(SendStatus &stat);


    /**
     * Handles the response from the server regarding a reques to receive 
     * messages
     * @param[out] msgs - holds the messages received from the server
     * @returns true if the response was successfully parsed
     */
    bool handleRespRecv(vector<Msg> &msgs);

private:
    /**
     * Ensures that the packet received from the server matches the expected 
     * packet type.
     * @param[in] expected_type - packet type expected the raw data to contain
     * @returns true if the inputted type matches raw data in rx buffer
     */
    bool validateRespHeader(const PacketType &expected_type);

    vector<uint8_t>& tx_buffer_; // ref. to buffer where encoded packets are stored
    vector<uint8_t>& rx_buffer_; // ref. to buffer where raw data is received from server
};