/**
* Database is a Singleton database for the server.
* Data here persists beyond a client/server request/response cycle.
* 
* TODO: handle concurrent accesses
*/
#pragma once

#include "common/common.h"

#include <functional>
#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>

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

class Database {
public:
    /**
     * Iterator class allows for iteration through the unordered_map which is 
     * the heart of the database
     */
    class ConstIterator {
    public:
        ConstIterator(unordered_map<user_id_t, UserMetadata>::const_iterator it) : it_(it) {}
        const pair<const user_id_t, UserMetadata>& operator*() const {return *it_; }

        ConstIterator& operator++() {
            it_++;
            return *this;
        }

        bool operator!=(const ConstIterator& other) const { return other.it_ != it_; }

    private:
        unordered_map<user_id_t, UserMetadata>::const_iterator it_;
    };

    static Database& getInstance() {
        static Database s;
        return s;
    }

    ConstIterator begin() const { return ConstIterator(user_map_.cbegin());}
    ConstIterator end() const { return ConstIterator(user_map_.cend());}

    /**
     * Returns the next available user ID and assigns it the username string
     * TODO: generating a new user ID can be accomplished in other ways, such as
     * UUID, RNG and more.
     */
    user_id_t registerUser(const string& username);

    /**
     * Stores the message for the intended recepient for later access
     */
    bool queueMsg(user_id_t recepient_id, const Msg &m);

    /**
     * TODO: add function to delete a user. Perhaps this would happen after a 
     * certain timeout period after client has not heard from the server
     * 
     * For debugging: prints the content of the database
     */
    void print();

    /**
     * Returns true a user ID exists in the database
     */
    bool userExists(user_id_t id) const{
        return user_map_.contains(id);
    }

    user_id_t getNumUsers() const {
        return user_map_.size(); // total number of users in the database
    }

    UserMetadata& operator[](user_id_t id) {
        return user_map_[id];
    }

    /**
     * Given a user ID, returns the corresponding username
     */
    string getUsername(user_id_t id) {
        return user_map_.contains(id) ? user_map_[id].username : "";
    }

private:
    Database(){} // user may only access class through getInstance
                    // private constructor supports the Singleton paradigm

    user_id_t next_id_{0U}; // stores the next user ID to create in database
    // stores all user data - maps a user ID to user metadata
    unordered_map<user_id_t, UserMetadata> user_map_; 
};

