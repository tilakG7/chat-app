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

    static void registerUser(App *my_app) {
        Socket my_socket;
        my_socket.connectB("127.0.0.1", 8080);

        // get username
        my_app->username_ = my_app->console_.read("Hi, there. Enter your name to start chatting >");
        // send request to register user
        size_t num_bytes_to_send = my_app->my_client_.encodeRequestRegister(my_app->username_);
        uint8_t *p_tx_buffer_start = &my_app->tx_buffer_[0];
        uint8_t *p_rx_buffer_start = &my_app->rx_buffer_[0];
        my_socket.sendB(reinterpret_cast<char*>(p_tx_buffer_start), num_bytes_to_send);
        // get response from server
        while(my_socket.receiveNb(reinterpret_cast<char*>(p_rx_buffer_start), 1000U) == 0U) {}

        if(!my_app->my_client_.handleRespRegister(my_app->id_)) {
            my_app->console_.write("Error in response from server within registerUser function");
        }

        cout << "Successfully received user ID: " << my_app->id_;
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

    void displayOnlineUsers() {

    }



    void chatWithUser() {
        string target_user = "";
        while(target_user == "") {
            target_user = console_.read("Enter the name of the user you would like to chat with: ");
            if(!name_to_id.contains(target_user)) {
                console_.write("Error,  user not available");
                target_user = "";
            }
        }


        while(true) {
            string msg = console_.read("Enter message to send to " + target_user + "or X to exit> ");
            if(msg[0] == 'X') {
                return;
            }
            // TODO: send msg
        }
    }

    void run() {
        thread t1(registerUser, this);
        t1.join();
        
        while(true) {
            switch(getUserCommand()) {
                case Command::kDisplayOnlineUsers:
                    cout << "You selected kGetOnlineUsers" << endl;
                break;
                case Command::kChatWithUser:
                    cout << "You selected kChatWithUser" << endl;
                break;
                default:
                break;
            }
        }
    }
private:
    Console &console_;
    unordered_map<string, user_id_t> name_to_id; // maps username to id
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