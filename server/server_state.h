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
    /**
     * Returns the next available user ID and assigns it the username string
     * TODO: generating a new user ID can be accomplished in other ways, such as
     * UUID, RNG and more.
     */
    user_id_t registerUser(const string& username) {
        user_map_[next_id_] = {username, {}};
        return next_id_++;
    }

    /**
     * Stores the message for the intended recepient for later access
     */
    bool queueMsg(user_id_t recepient_id, Msg &m) {
        // user intended to be the recepient of the message must exist
        if(user_map_.find(recepient_id) == user_map_.end()) {
            return false;
        }
        user_map_[recepient_id].msg_q.push_back(m); // enqueue message
        return true;
    }

    /**
     * TODO: add function to delete a user. Perhaps this would happen after a 
     * certain timeout period after client has not heard from the server
     */

    void print() {
        for(auto group : user_map_) {
            cout << "User ID: " << group.first << endl;
            cout << "Username : " << group.second.username << endl;
            size_t msg_count = 1;
            for(auto msg : group.second.msg_q) {
                cout << "Message #" << msg_count << ": " << endl;
                cout << "Sender ID: " << msg.sender_id << endl;
                cout << "Message: " << msg.msg << endl << endl;
            }
            cout << "--------------------------------";
        }
    }

private:
    user_id_t next_id_{0U}; // stores the next user ID
    // stores all user data - maps a user ID to user metadata
    unordered_map<user_id_t, UserMetadata> user_map_; 
};

int main() {
    ServerState ss;
    user_id_t user_a = ss.registerUser("Jack");
    user_id_t user_b = ss.registerUser("Jill");
    ss.print();
    Msg msg_a{user_a, "Hi Jill, how are ya?"};
    ss.queueMsg(user_b, msg_a);
    Msg msg_b{user_b, "Hi Jack, how are ya?"};
    ss.queueMsg(user_a, msg_b);
    ss.print();
}
