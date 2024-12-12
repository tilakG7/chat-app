#include "database.h"

#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>

using namespace std;

user_id_t Database::registerUser(const string& username) {
    user_map_[next_id_] = {username, {}};
    return next_id_++;
}

bool Database::queueMsg(user_id_t recepient_id, const Msg &m) {
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
void Database::print() {
    for(auto group : user_map_) {
        cout << "User ID: " << group.first << endl;
        cout << "Username : " << group.second.username << endl;
        size_t msg_count = 1;
        for(auto msg : group.second.msg_q) {
            cout << "Message #" << msg_count << ": " << endl;
            cout << "Sender ID: " << msg.sender_id << endl;
            cout << "Message: " << msg.msg << endl << endl;
        }
        cout << "--------------------------------" << endl;
    }
}


// int main() {
//     Database ss;
//     user_id_t user_a = ss.registerUser("Jack");
//     user_id_t user_b = ss.registerUser("Jill");
//     ss.print();
//     Msg msg_a{user_a, "Hi Jill, how are ya?"};
//     ss.queueMsg(user_b, msg_a);
//     Msg msg_b{user_b, "Hi Jack, how are ya?"};
//     ss.queueMsg(user_a, msg_b);
//     ss.print();
// }
