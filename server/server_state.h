/**
* ServerState class acts as a database for the server-side messaging 
* application.
* Data here persists beyond a client/server request/response cycle.
*/

#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>

using user_id_t = uint32_t; // type to hold user IDs
using namespace std;


// Stores all message details to be forwarded to the recipient of the message
struct Msg {
    user_id_t sender_id;
    string msg;
};

// Data persisted for each user
struct UserMetadata {
    string username;
    vector<Msg> msg_q;
};

class ServerState {
public:
    static ServerState& getInstance() {
        static ServerState s;
        return s;
    }
    
    /**
     * Returns the next available user ID and assigns it the username string
     * TODO: generating a new user ID can be accomplished in other ways, such as
     * UUID, RNG and more.
     */
    user_id_t registerUser(const string& username);

    /**
     * Stores the message for the intended recepient for later access
     */
    bool queueMsg(user_id_t recepient_id, Msg &m);

    /**
     * TODO: add function to delete a user. Perhaps this would happen after a 
     * certain timeout period after client has not heard from the server
     */
    void print();

private:
    ServerState(){} // private constructor

    user_id_t next_id_{0U}; // stores the next user ID
    // stores all user data - maps a user ID to user metadata
    unordered_map<user_id_t, UserMetadata> user_map_; 
};

