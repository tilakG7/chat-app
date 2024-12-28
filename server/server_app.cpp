#include <iostream>
#include <thread>
#include <vector>

#include "database.h"
#include "server.h" // for MCC layer parsing
#include "tcp_lib/server_socket.h"
#include "tcp_lib/socket.h"


class ServerApp {
public:
    ServerApp(string ip, int port) : ip_(ip), port_(port){}
    
    /**
     * Receives raw bytes and gives it to the upper MCC layer to parse.
     * Sends the response given from the upper layer
     * Function is called as a seperate thread for each incoming request to the 
     * server.
     */
     static void handleRequest(int socket_descriptor) {
        Socket my_socket(socket_descriptor, true);

        vector<uint8_t> rx_buffer(1024); // 1KB buffer to receive data from client
        vector<uint8_t> tx_buffer(1024); // 1KB buffer to store response from MCC layer
        MccServer mcc(Database::getInstance(), tx_buffer);

        // receive data
        size_t num_bytes_rx = 0;
        do {
            num_bytes_rx = my_socket.receiveNb(reinterpret_cast<char*>(&rx_buffer[0]), rx_buffer.size());
        } while(num_bytes_rx == 0);
        
        // parse request and get size of response
        size_t num_bytes_tx = mcc.parse(&rx_buffer[0], num_bytes_rx);

        // send response
        my_socket.sendB(reinterpret_cast<char*>(&tx_buffer[0]), num_bytes_tx);
    }

    /**
     * Creates a listening socket for the given IP and port
     * Gets new connections and creates a thread to handle the requests
     */
    void run() {
        ServerSocket my_server_socket(ip_, port_);
        while(true) {
            optional<int> opt_socket_descriptor;
            while(!(opt_socket_descriptor = my_server_socket.getConnection()).has_value()) {}
            std::thread t(ServerApp::handleRequest, *opt_socket_descriptor);
            t.detach();
        }
    }
private:
    string ip_; // ip address of the server
    int port_;   // port that the server listens on
};


int main(int argc, char *argv[]) {
    ServerApp my_server(argv[1], stoi(argv[2]));
    my_server.run();
    return 0;

}
// c++ -std=c++20 server_app.cpp database.cpp server.cpp -I . -I ../ -o b && ./b 127.0.0.1 8080      