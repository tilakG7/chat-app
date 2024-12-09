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
    class ConstIterator {
    public:
        ConstIterator(const unordered_map<user_id_t, UserMetadata>& um, bool end = false) {
            if(end) {
                it_ = um.cend();
                return;
            }
            it_ = um.cbegin();
        }

        const pair<user_id_t, UserMetadata>& operator*() const {return *it_; }
        
        ConstIterator& operator++() {
            it_++;
            return *this;
        }

        bool operator!=(const ConstIterator& other) const {
            return other.it_ != it_;
        }

    private:
        unordered_map<user_id_t, UserMetadata>::const_iterator it_;

    };

    static ServerState& getInstance() {
        static ServerState s;
        return s;
    }

    ConstIterator begin() const { return ConstIterator(user_map_);}
    ConstIterator end() const { return ConstIterator(user_map_, true);}

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

    /**
     * Whether a user ID exists in storage
     */
    bool userExists(user_id_t id) {
        return user_map_.contains(id);
    }

private:
    ServerState(){} // private constructor

    user_id_t next_id_{0U}; // stores the next user ID
    // stores all user data - maps a user ID to user metadata
    unordered_map<user_id_t, UserMetadata> user_map_; 
};

