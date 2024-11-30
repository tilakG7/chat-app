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
private:
    unordered_map<user_id_t, string> id_to_user_; // maps a user ID to a username
};
