/**
* ServerState class acts as a database for the server-side messaging 
* application.
* Data here persists beyond a client/server request/response cycle.
* 
*/

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
    // TODO: generating a new user ID can be accomplished in other ways
    user_id_t registerUser(const string& username) {
        user_map_[next_id_] = {username, {}};
        return next_id_++;
    }

private:
    user_id_t next_id_{0U}; // stores the next user ID
    unordered_map<user_id_t, UserMetadata> user_map_; // maps a user ID to user metadata
};
