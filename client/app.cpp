#include "app.h"
#include "client.h"
#include "client_parser.h"

#include "console/console.h"
#include "common/common.h"
#include "tcp_lib/socket.h"

#include <iostream>
#include <thread>

using namespace std::chrono_literals;


void App::registerUser() {
    Socket my_socket;
    my_socket.connectB(kServerIp, kServerPort); // connect to server

    // get username from user
    username_ = console_.read("Hi, there. Enter your name to start chatting >");
    // send request to register user
    vector<uint8_t> tx_buffer = vector<uint8_t>(1000);
    vector<uint8_t> rx_buffer = vector<uint8_t>(1000);
    MccClient my_client(tx_buffer, rx_buffer);
    // size_t num_bytes_to_send = my_client.encodeRequestRegister(username_);
    string tx_str = ClientParser::encodeRequestRegister(username_);

    uint8_t *p_rx_buffer_start = &rx_buffer[0];
    my_socket.sendB(tx_str.data(), tx_str.size());
    // get response from server
    while(my_socket.receiveNb(reinterpret_cast<char*>(p_rx_buffer_start), 1000U) == 0U) {}

    if(!my_client.handleRespRegister(id_)) {
        console_.write("ERR: unexpected server resp in registerUser");
        return;
    }
}

Command App::getUserCommand() {
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

void App::periodicGetOnlineUsers() {
    vector<uint8_t> rx_buffer(1000, 0);
    vector<uint8_t> tx_buffer(1000, 0);
    MccClient my_client{tx_buffer, rx_buffer};

    while(true) {
        std::this_thread::sleep_for(1s);
        Socket my_socket;
        my_socket.connectB(kServerIp, kServerPort); // connect to server

        size_t num_bytes_to_send = my_client.encodeRequestUsers(id_);
        
        my_socket.sendB(reinterpret_cast<char*>(&tx_buffer[0]), num_bytes_to_send);
        // get response from server
        while(my_socket.receiveNb(reinterpret_cast<char*>(&rx_buffer[0]), 1000U) == 0U) {}

        unordered_map<string, user_id_t> new_name_to_id;
        if(!my_client.handleRespUsers(new_name_to_id)) {
            console_.write("ERR: unexpected server resp in taskGetOnlineUsers");
            return;
        }

        {
            unique_lock<mutex> lck(name_mtx);
            // if the new mapping is the same as old, the status of online users has not
            // changed, thus, we don't need to do anything
            // todo: make this comparison better
            if(new_name_to_id == name_to_id_) {
                continue;
            }

            name_to_id_ = new_name_to_id;
            // copy contents to id_to_name_
            id_to_name_.clear();
            for(auto mapping : name_to_id_) {
                id_to_name_.emplace(mapping.second, mapping.first);
            }

            console_.write("**********Updated Online users: ");

            if(!name_to_id_.size()) {
                console_.write("No other users currently online");
                continue;
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
    }
}

void App::printOnlineUsers() {
    {
        unique_lock<mutex> lck(name_mtx);
        // print the names of all users
        for(const auto& mapping : name_to_id_) {
            if(mapping.second == id_) {
                continue; // skip current user
            }
            console_.write(mapping.first); // write name of user
            // @todo: also add code to print user ID for debugging purposes
        }
    }
}



void App::taskChatWithUser() {
    string target_user = "";
    while(target_user == "") {
        target_user = console_.read("Enter the name of the user you would like to chat with: ");
        // allow user to exit chatting with a user if they put x
        // todo: ensure username is not x
        if(target_user == "x") {
            return;
        }
        {
            unique_lock<mutex> lck(name_mtx);
            if(!name_to_id_.contains(target_user)) {
                console_.write("Error,  user not available");
                target_user = "";
            } else {
                break;
            }
        }
    }

    vector<uint8_t> tx_buffer(1000, 0);
    vector<uint8_t> rx_buffer(1000, 0);
    MccClient my_client(tx_buffer, rx_buffer);
    while(true) {
        string msg = console_.read("Send message to " + target_user + ">" );
        if(msg == "x") {
            return; // exit chatting
        }
        Socket my_socket;
        my_socket.connectB(kServerIp, kServerPort); // connect to server

        size_t num_bytes_to_send;
        {
            unique_lock<mutex> lck(name_mtx);
            num_bytes_to_send = my_client.encodeRequestSend(id_, name_to_id_[target_user], msg);
        }
        uint8_t *p_tx_buffer_start = &tx_buffer[0];
        uint8_t *p_rx_buffer_start = &rx_buffer[0];
        my_socket.sendB(reinterpret_cast<char*>(p_tx_buffer_start), num_bytes_to_send);
        // get response from server
        while(my_socket.receiveNb(reinterpret_cast<char*>(p_rx_buffer_start), 1000U) == 0U) {}
        
        SendStatus stat;
        if(!my_client.handleRespSend(stat)) {
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

void App::periodicGetMessages(){ 
    vector<uint8_t> rx_buffer(1000, 0);
    vector<uint8_t> tx_buffer(1000, 0);
    MccClient my_client{tx_buffer, rx_buffer};

    while(true) {
        std::this_thread::sleep_for(1s);

        Socket my_socket;
        my_socket.connectB(kServerIp, kServerPort); // connect to server

        size_t num_bytes_to_send = my_client.encodeRequestRecv(id_);
        uint8_t *p_tx_buffer_start = &tx_buffer[0];
        uint8_t *p_rx_buffer_start = &rx_buffer[0];
        my_socket.sendB(reinterpret_cast<char*>(p_tx_buffer_start), num_bytes_to_send);
        // get response from server
        while(my_socket.receiveNb(reinterpret_cast<char*>(p_rx_buffer_start), 1000U) == 0U) {}
        
        vector<Msg> msgs;
        if(!my_client.handleRespRecv(msgs)) {
            Console::getInstance().write("ERR: server response to receive message not handled");
            return;
        }


        for(auto msg : msgs) {
            string to_print = "";
            if(id_to_name_.contains(msg.sender_id)) {
                to_print += id_to_name_[msg.sender_id];
                to_print += "> ";
            } else {
                to_print += "Unknown sender > ";
            }
            to_print += msg.msg;
            Console::getInstance().write(to_print);
        }
    }
}


void App::run() {
    registerUser();
    thread t1(&App::periodicGetMessages, this);
    thread t2(&App::periodicGetOnlineUsers, this);
    while(true) {
        switch(getUserCommand()) {
            case Command::kDisplayOnlineUsers:
                printOnlineUsers();
                break;
            case Command::kChatWithUser:
                taskChatWithUser();
            break;
            default:
            break;
        }
    }
    t1.join();
    t2.join();
}
int main() {
    string str_ip = Console::getInstance().read("Enter the server IP >");
    string str_port = Console::getInstance().read("Enter the server port >");

    App app(str_ip, stoi(str_port));
    app.run();
}

// clear && c++ -std=c++20 app.cpp client.cpp ../console/console.cpp -I . -I ../ -o b && ./b