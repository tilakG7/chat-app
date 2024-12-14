#include "console/console.h"
#include <iostream>

class App {
public:
    App() : console_(Console::getInstance()) {}

    // this function is called when user starts chatting
    void chatting() {
        
    }
    void run() {
        // get username
        string username = console_.read("Hi, there. Enter your name to start chatting >");
        // TODO: send request to register user

        console_.write("Hi " + username + "! Follow the instructions below:");
        console_.write("Type \"1\" for viewing online users");
        console_.write("Type \"2\" followed by a username to chat with that user");
        string user_cmd = console_.read("Enter your choice >");
        // parse user_cmd
        // if 1, then issue a request online users request
        // if 2, then ensure the username they typed in is correct

    }
private:
    Console &console_;

};

int main() {
    App app;
    app.run();
    // enter your name, register name with server
    // show current online users
    // periodically check for new messages
    // ask user who they want to chat with
}