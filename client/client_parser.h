/**
 * Encodes and decodes raw byte data
 * Encodes requests from the client
 * Decodes responses from the server
 */
#pragma once

#include "common/common.h"

#include <sstream>
#include <string>

using namespace std;

class ClientParser {
public:
    ClientParser() = default;

    /**
     * Encodes a request to register a user
     * @param username - string representing the username to register
     * @return # of bytes encoded
     */
    static string encodeRequestRegister(const string& username);

    // /**
    //  * Handles the response to the register user request
    //  * @param[out] id - holds the id of the newly registered user
    //  * @returns true if the response was handled successfully
    //  */
    // bool handleRespRegister(user_id_t &id);
};