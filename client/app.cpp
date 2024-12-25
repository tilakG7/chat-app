#include "app.h"
#include "client.h"

#include "console/console.h"
#include "common/common.h"
#include "tcp_lib/socket.h"

#include <iostream>
#include <thread>

class App {
public:
    App() : console_(Console::getInstance()) {
    }

    /**
     * Registers user with server and stores the user ID assigned by server 
     * response
     */
    void taskRegisterUser() {
        Socket my_socket;
        my_socket.connectB(kServerIp, kServerPort); // connect to server

        // get username from user
        username_ = console_.read("Hi, there. Enter your name to start chatting >");
        // send request to register user
        size_t num_bytes_to_send = my_client_.encodeRequestRegister(username_);
        uint8_t *p_tx_buffer_start = &tx_buffer_[0];
        uint8_t *p_rx_buffer_start = &rx_buffer_[0];
        my_socket.sendB(reinterpret_cast<char*>(p_tx_buffer_start), num_bytes_to_send);
        // get response from server
        while(my_socket.receiveNb(reinterpret_cast<char*>(p_rx_buffer_start), 1000U) == 0U) {}

        if(!my_client_.handleRespRegister(id_)) {
            console_.write("ERR: unexpected server resp in taskRegisterUser");
            return;
        }

        cout << "Successfully received user ID: " << id_;
    }

    Command getUserCommand() {
        console_.write("Hi " + username_ + "! Follow the instructions below:");
        console_.write("Type \"1\" for viewing online users");
        console_.write("Type \"2\" to chat with a user");

        static unordered_map<char, Command> cmd_map{
            {'1', Command::kDisplayOnlineUsers},
            {'2', Command::kChatWithUser}
        };

        while(true) {
            string user_cmd = console_.read("Enter your choice >");

            if(user_cmd.size() == 0 || user_cmd.size() > 1 || !cmd_map.contains(user_cmd[0])) {
                console_.write("Invalid selection, try again");
            } else {
                return cmd_map[user_cmd[0]];
            }
        }
    }

    /**
     * Gets online users from the server and populates local cache
     */
    void taskGetOnlineUsers() {
        Socket my_socket;
        my_socket.connectB(kServerIp, kServerPort); // connect to server

        size_t num_bytes_to_send = my_client_.encodeRequestUsers(id_);
        uint8_t *p_tx_buffer_start = &tx_buffer_[0];
        uint8_t *p_rx_buffer_start = &rx_buffer_[0];
        my_socket.sendB(reinterpret_cast<char*>(p_tx_buffer_start), num_bytes_to_send);
        // get response from server
        while(my_socket.receiveNb(reinterpret_cast<char*>(p_rx_buffer_start), 1000U) == 0U) {}

        if(!my_client_.handleRespUsers(name_to_id_)) {
            console_.write("ERR: unexpected server resp in taskGetOnlineUsers");
            return;
        }

        console_.write("Online users: ");

        if(name_to_id_.size() == 1) {
            console_.write("No other users currently online");
            return;
        }

        // print the names of all users
        for(const auto& mapping : name_to_id_) {
            if(mapping.second == id_) {
                continue; // skip current user
            }
            console_.write(mapping.first); // write name of user
            // @todo: also add code to print user ID for debugging purposes
        }
    }



    void taskChatWithUser() {
        string target_user = "";
        while(target_user == "") {
            target_user = console_.read("Enter the name of the user you would like to chat with: ");
            if(!name_to_id_.contains(target_user)) {
                console_.write("Error,  user not available");
                target_user = "";
            } else {
                break;
            }
        }

        while(true) {
            string msg = console_.read("Send message to " + target_user + ">" );
            if(msg == "x") {
                return; // exit chatting
            }
            Socket my_socket;
            my_socket.connectB(kServerIp, kServerPort); // connect to server
            size_t num_bytes_to_send = my_client_.encodeRequestSend(id_, name_to_id_[target_user], msg);
            uint8_t *p_tx_buffer_start = &tx_buffer_[0];
            uint8_t *p_rx_buffer_start = &rx_buffer_[0];
            my_socket.sendB(reinterpret_cast<char*>(p_tx_buffer_start), num_bytes_to_send);
            // get response from server
            while(my_socket.receiveNb(reinterpret_cast<char*>(p_rx_buffer_start), 1000U) == 0U) {}
            
            SendStatus stat;
            if(!my_client_.handleRespSend(stat)) {
                console_.write("ERR: unexpected server resp in taskChatWithUser");
                return;
            }

            switch(stat) {
                case SendStatus::kSuccess:
                    console_.write("Message succesfully sent");
                break;
                case SendStatus::kErrSrcId:
                    console_.write("ERR: Source ID incorrect in send request");
                break;
                case SendStatus::kErrTrgtId:
                    console_.write("ERR: Target ID incorrect in send request");
                break;
                default:
                    console_.write("ERR: Received unexpected response value from server in Send request");
                break;
            }
        }
    }


    //     while(true) {
    //         string msg = console_.read("Enter message to send to " + target_user + "or X to exit> ");
    //         if(msg[0] == 'X') {
    //             return;
    //         }
    //         // TODO: send msg
    //     }
    // }

    void run() {
        taskRegisterUser();
        while(true) {
            switch(getUserCommand()) {
                case Command::kDisplayOnlineUsers:
                    taskGetOnlineUsers();
                    break;
                case Command::kChatWithUser:
                    taskChatWithUser();
                break;
                default:
                break;
            }
        }
    }
private:
    Console &console_;
    unordered_map<string, user_id_t> name_to_id_; // maps username to id
    string username_; // name of user
    user_id_t id_; // id of current user 

    vector<uint8_t> tx_buffer_ = vector<uint8_t>(1000);
    vector<uint8_t> rx_buffer_ = vector<uint8_t>(1000);
    MccClient my_client_{tx_buffer_, rx_buffer_};

};

int main() {
    App app;
    app.run();
}

// c++ -std=c++20 app.cpp client.cpp ../console/console.cpp -I . -I ../ -o b && ./b