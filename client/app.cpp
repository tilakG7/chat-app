#include "app.h"
#include "client.h"
#include "console/console.h"
#include "common/common.h"
#include <iostream>

class App {
public:
    App() : console_(Console::getInstance()) {
    }

    void registerUser() {
        // get username
        username_ = console_.read("Hi, there. Enter your name to start chatting >");
        // TODO: send request to register user
        my_client_.encodeRequestRegister(username_);
        // actually send it
        // get response
        // handle response
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
        registerUser();
        
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
    vector<uint8_t> tx_buffer_(1000U);
    vector<uint8_t> rx_buffer_(1000U);
    MccClient my_client_(tx_buffer_);

};

int main() {
    App app;
    app.run();
}