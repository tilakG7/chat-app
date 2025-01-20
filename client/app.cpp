#include "app.h"
#include "client.h"
#include "client_parser.h"

#include "console/console.h"
#include "common/common.h"
#include "common/packet.pb.h"
#include "tcp_lib/socket.h"

#include <iostream>
#include <thread>

using namespace std::chrono_literals;


void App::registerUser() {
    Socket my_socket;
    my_socket.connectB(kServerIp, kServerPort); // connect to server

    // get username from user
    username_ = console_.read("Hi, there. Enter your name to start chatting >");

    mcc::Packet req_packet;
    req_packet.mutable_hdr()->set_packet_type(mcc::Header::PACKET_TYPE_REQUEST_REGISTER);
    req_packet.mutable_req_reg()->set_username(username_);

    vector<char> tx_buffer(1000);
    vector<uint8_t> rx_buffer(1000);

    // send request to register user
    !req_packet.SerializeToArray(tx_buffer.data(), tx_buffer.size());
    my_socket.sendB(&tx_buffer[0], req_packet.ByteSizeLong());

    // receive response from the user
    while(my_socket.receiveNb(reinterpret_cast<char*>(&rx_buffer[0]), 1000U) == 0U) {}

    mcc::Packet resp_packet;
    resp_packet.ParseFromArray(rx_buffer.data(), rx_buffer.size());

    // ensure response is valid
    if(!resp_packet.has_hdr()) {
        cerr << "Error. Register user response is missing a header" << endl;
    }
    if(!resp_packet.has_resp_reg() || !resp_packet.resp_reg().has_resp_value()) {
        cerr << "Error. Register user response is missing response message" << endl;
    }
    if(resp_packet.resp_reg().resp_value() != 0) {
        cerr << "Error. Server responded with a negative response for registering the user." << endl;
    }
    id_ = resp_packet.resp_reg().assigned_id();
    console_.write("DEBUG: Server assigned user ID = " + to_string(id_));
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

        if(user_cmd.size() == 0 || user_cmd.size() > 1 || (cmd_map.find(user_cmd[0]) == cmd_map.end())) {
            console_.write("Invalid selection, try again");
        } else {
            return cmd_map[user_cmd[0]];
        }
    }
}

void App::periodicGetOnlineUsers() {
    vector<char> rx_buffer(1000, 0);
    vector<char> tx_buffer(1000, 0);

    while(true) {
        std::this_thread::sleep_for(1s);
        Socket my_socket;
        my_socket.connectB(kServerIp, kServerPort); // connect to server

        mcc::Packet req_packet;
        req_packet.mutable_hdr()->set_packet_type(mcc::Header::PACKET_TYPE_REQUEST_USERS);
        req_packet.mutable_req_users()->set_requestor_id(id_);
        req_packet.SerializeToArray(tx_buffer.data(), tx_buffer.size());
        size_t num_bytes_to_send = req_packet.ByteSizeLong();
        
        my_socket.sendB(tx_buffer.data(), num_bytes_to_send);
        // get response from server
        while(my_socket.receiveNb(rx_buffer.data(), 1000U) == 0U) {}

        mcc::Packet resp_packet;
        resp_packet.ParseFromArray(rx_buffer.data(), rx_buffer.size());

        if(resp_packet.resp_users().resp_value() != 0) {
            console_.write("DEBUG: ERR, negative response for get online users request");
            continue;
        }

        unordered_map<string, user_id_t> new_name_to_id;
        for(size_t i=0; i < resp_packet.resp_users().online_users_size(); i++) {
            const mcc::OnlineUser user = resp_packet.resp_users().online_users(i);
            new_name_to_id[user.username()] = user.user_id();
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

        }
        printOnlineUsers();
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
            if(name_to_id_.find(target_user) == name_to_id_.end()) {
                console_.write("Error,  user not available");
                target_user = "";
            } else {
                break;
            }
        }
    }

    vector<char> tx_buffer(1000, 0);
    vector<char> rx_buffer(1000, 0);

    while(true) {
        string msg = console_.read("Send message to " + target_user + ">" );
        if(msg == "x") {
            return; // exit chatting
        }
        Socket my_socket;
        my_socket.connectB(kServerIp, kServerPort); // connect to server


        mcc::Packet req_packet;
        req_packet.mutable_hdr()->set_packet_type(mcc::Header::PACKET_TYPE_REQUEST_SEND);
        req_packet.mutable_req_send()->set_message(msg);
        req_packet.mutable_req_send()->set_source_id(id_);
        {
            unique_lock<mutex> lck(name_mtx);
            req_packet.mutable_req_send()->set_target_id(name_to_id_[target_user]);
        }
        req_packet.SerializeToArray(tx_buffer.data(), tx_buffer.size());
        my_socket.sendB(tx_buffer.data(), req_packet.ByteSizeLong());
        
        // get response from server
        while(my_socket.receiveNb(rx_buffer.data(), rx_buffer.size()) == 0U) {}
        
        mcc::Packet resp_packet;
        resp_packet.ParseFromArray(rx_buffer.data(), rx_buffer.size());
        if(!resp_packet.resp_send().has_resp_value()) {
            console_.write("ERR: unexpected server response for sending a message");
            return;
        }

        switch(static_cast<SendStatus>(resp_packet.resp_send().resp_value())) {
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
    vector<char> rx_buffer(1000, 0);
    vector<char> tx_buffer(1000, 0);

    while(true) {
        std::this_thread::sleep_for(1s);

        Socket my_socket;
        my_socket.connectB(kServerIp, kServerPort); // connect to server

        mcc::Packet req_packet;
        req_packet.mutable_hdr()->set_packet_type(mcc::Header::PACKET_TYPE_REQUEST_RECV);
        req_packet.mutable_req_recv()->set_requestor_id(id_);
        req_packet.SerializeToArray(tx_buffer.data(), tx_buffer.size());
        size_t num_bytes_to_send = req_packet.ByteSizeLong();
        my_socket.sendB(tx_buffer.data(), num_bytes_to_send);
        // get response from server
        while(my_socket.receiveNb(reinterpret_cast<char*>(rx_buffer.data()), 1000U) == 0U) {}
        
        mcc::Packet resp_packet;
        resp_packet.ParseFromArray(rx_buffer.data(), rx_buffer.size());
        for(size_t i=0; i < resp_packet.resp_recv().messages_size(); i++) {
            const mcc::UserMessage& msg = resp_packet.resp_recv().messages(i);

            string to_print = "";
            if(id_to_name_.find(msg.user_id()) != id_to_name_.end()) {
                to_print += id_to_name_[msg.user_id()];
                to_print += "> ";
            } else {
                to_print += "Unknown sender > ";
            }
            to_print += msg.message();
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
    // string str_ip = Console::getInstance().read("Enter the server IP >");
    // string str_port = Console::getInstance().read("Enter the server port >");
    string str_ip = "127.0.0.1";
    string str_port = "8080";

    App app(str_ip, stoi(str_port));
    app.run();
}

// clear && c++ -std=c++20 app.cpp client.cpp ../console/console.cpp -I . -I ../ -o b && ./b