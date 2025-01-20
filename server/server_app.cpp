#include <iostream>
#include <thread>
#include <vector>

#include "common/packet.pb.h"
#include "database.h"
#include "server.h" // for MCC layer parsing
#include "tcp_lib/server_socket.h"
#include "tcp_lib/socket.h"


class ServerApp {
public:
    ServerApp(string ip, int port) : ip_(ip), port_(port){}

    static size_t handleRequestRegister(const mcc::Packet &packet, char *tx_buffer, size_t capacity) {
        if(!packet.has_req_reg() || !packet.req_reg().has_username()) {
            cerr << "Invalid request register from client" << endl;
            return 0;
        }
        
        // unique ID assigned to user
        user_id_t id = Database::getInstance().registerUser(packet.req_reg().username()); 

        mcc::Packet resp_packet;
        resp_packet.mutable_hdr()->set_packet_type(mcc::Header::PACKET_TYPE_RESP_REGISTER);
        resp_packet.mutable_resp_reg()->set_resp_value(0);
        resp_packet.mutable_resp_reg()->set_assigned_id(id);

        resp_packet.SerializeToArray(tx_buffer, capacity);
        return resp_packet.ByteSizeLong();
    }


    static size_t handleRequestUsers(const mcc::Packet &packet, char *tx_buffer, size_t capacity) {
        if(!packet.has_req_users() || !packet.req_users().has_requestor_id()) {
            cerr << "Invalid request users from client" << endl;
            return 0;
        }

        mcc::Packet resp_packet;
        resp_packet.mutable_hdr()->set_packet_type(mcc::Header::PACKET_TYPE_RESP_USERS);
        // return a negative response if the user does not exist in the database
        if(!Database::getInstance().userExists(packet.req_users().requestor_id())) {
            resp_packet.mutable_resp_users()->set_resp_value(1); // todo: replace magic number
        } else {
            resp_packet.mutable_resp_users()->set_resp_value(0); // todo: replace magic number
            for(auto &it : Database::getInstance()) {
                // current user should only know which OTHER users are online
                if(it.first == packet.req_users().requestor_id()) {
                    continue;
                }

                mcc::OnlineUser *online_user = resp_packet.mutable_resp_users()->add_online_users();
                online_user->set_user_id(it.first);
                online_user->set_username(it.second.username);
            }
        }
        resp_packet.SerializeToArray(tx_buffer, capacity);
        return resp_packet.ByteSizeLong();   
    }

    static size_t handleRequestRecv(const mcc::Packet &packet, char *tx_buffer, size_t capacity) {
        if(!packet.has_req_recv() || !packet.req_recv().has_requestor_id()) {
            cerr << "Invalid request to receive messages from client" << endl;
            return 0;
        }

        mcc::Packet resp_packet;
        resp_packet.mutable_hdr()->set_packet_type(mcc::Header::PACKET_TYPE_RESP_RECV);

        // Return a negative response if user ID not in server storage
        if(!Database::getInstance().userExists(packet.req_recv().requestor_id())) {
            resp_packet.mutable_resp_recv()->set_resp_value(1); // todo: remove magic number
        } else {
            resp_packet.mutable_resp_recv()->set_resp_value(0); // todo: remove magic number
            
            for(Msg &m : Database::getInstance()[packet.req_recv().requestor_id()].msg_q) {
                mcc::UserMessage *um = resp_packet.mutable_resp_recv()->add_messages();
                um->set_user_id(m.sender_id);
                um->set_message(m.msg);
            }
        }

        resp_packet.SerializeToArray(tx_buffer, capacity);
        return resp_packet.ByteSizeLong();
    }
    
    /**
     * Receives raw bytes and gives it to the upper MCC layer to parse.
     * Sends the response given from the upper layer
     * Function is called as a seperate thread for each incoming request to the 
     * server.
     */
     static void handleRequest(int socket_descriptor) {
        Socket my_socket(socket_descriptor, true);

        vector<char> tx_buffer(1024);
        vector<char> rx_buffer(1024); // 1KB buffer to receive data from client

        // receive data
        size_t num_bytes_rx = 0;
        do {
            num_bytes_rx = my_socket.receiveNb(&rx_buffer[0], rx_buffer.size());
        } while(num_bytes_rx == 0);

        mcc::Packet packet;
        packet.ParseFromArray(rx_buffer.data(), 1024);

        if(!packet.has_hdr() || !packet.hdr().has_packet_type()) {
            cerr << "Received a request with an invalid header" << endl;
        }

        size_t num_bytes_tx = 0;
        switch (packet.hdr().packet_type()){
            case mcc::Header::PACKET_TYPE_REQUEST_RECV:
            break;
            case mcc::Header::PACKET_TYPE_REQUEST_REGISTER:
                num_bytes_tx = handleRequestRegister(packet, tx_buffer.data(), tx_buffer.size());
            break;
            case mcc::Header::PACKET_TYPE_REQUEST_SEND:
            break;
            case mcc::Header::PACKET_TYPE_REQUEST_USERS:
                num_bytes_tx = handleRequestUsers(packet, tx_buffer.data(), tx_buffer.size());
            break;
            default:
                cerr << "Received an invalid packet type" << endl;
        }
        cout << packet.DebugString() << endl;

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
            int socket_descriptor = -1;
            while(!my_server_socket.getConnection(socket_descriptor)) {}
            std::thread t(ServerApp::handleRequest, socket_descriptor);
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