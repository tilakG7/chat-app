#pragma once

#include "client.h"

#include "console/console.h"
#include "common/common.h"
#include "tcp_lib/socket.h"

#include <cstdint>
#include <iostream>
#include <thread>

enum class Command : uint8_t{
    kDisplayOnlineUsers,
    kChatWithUser,
};


using namespace std::chrono_literals;

class App {
public:
    /**
     * Construct the application
     * @param ip - the IP where the server is expected to be running
     * @param port - the port where the server is running
     */
    App(const string &ip, uint16_t port) : 
        console_(Console::getInstance()), 
        kServerIp{ip}, 
        kServerPort{port} {}

    /**
     * Prompts the user for their username and sends a request to register the 
     * user. In case of error, logs the error and exits
     */
    void taskRegisterUser();

    /**
     * Prompts the user to enter a command to determine next step
     * @returns Command - the command the user chose
     */
    Command getUserCommand();

    /**
     * Gets online users from the server and populates local cache
     */
    void taskGetOnlineUsers();


    /**
     * Let's the user chat with another user
     */
    void taskChatWithUser();

    /**
     * Periodically checks for new messages for current user by sending a 
     * request to the server
     */
    static void taskGetMessages(user_id_t my_id, App *p_my_app);

    /**
     * Run the overall app
     */
    void run();

private:
    Console &console_;
    unordered_map<string, user_id_t> name_to_id_; // maps username to id
    unordered_map<user_id_t, string> id_to_name_; // maps id to username
    string username_; // name of user
    user_id_t id_; // id of current user 

    vector<uint8_t> tx_buffer_ = vector<uint8_t>(1000);
    vector<uint8_t> rx_buffer_ = vector<uint8_t>(1000);
    MccClient my_client_{tx_buffer_, rx_buffer_};

    const string kServerIp;
    const uint16_t kServerPort;
};


// clear && c++ -std=c++20 app.cpp client.cpp ../console/console.cpp -I . -I ../ -o b && ./b
