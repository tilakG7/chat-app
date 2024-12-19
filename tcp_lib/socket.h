#pragma once

#include <iostream>
#include <vector>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

/**
 * Utility class to send data via a connected socket
 */
class Socket {
public:
    Socket(int fd  = -1, bool connected = false) : fd_(fd), connected_(connected) {
        if(connected_) {
            return;
        }
        
        // get new internet stream socket from system
        fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if(fd_ < 0) {
            perror("Failed to create client socket");
        }

        // Set socket to non-blocking
        int flags = fcntl(fd_, F_GETFL, 0);
        if (flags < 0) {
            perror("fcntl failed to get fd_ status flags");
        }
        flags |= O_NONBLOCK;
        if (fcntl(fd_, F_SETFL, flags) < 0) {
            perror("fcntl failed to set file status flags");
        }
    }

    /**
     * Non-blocking call to connect to a listening socket
     * @param ip - string representing IP to connect to 
     * @param port - port to connect to 
     * @returns bool - whether the connection was successful
     */
    bool connectNb(const string& ip, const uint16_t port) {
        sockaddr_in address;
        address.sin_family = AF_INET;
        inet_pton(AF_INET, ip.c_str(), &address.sin_addr.s_addr);
        address.sin_port = htons(port);
        if(connect(fd_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1) {
            // if connection in progress, return
            if((errno == EALREADY) || (errno == EINPROGRESS)) {
                return false; 
            } 
            // another error occured
            else if(errno != EISCONN) {
                perror("Client failed to connect");
                return false;
            }
        }
        connected_ = true;
        return connected_;
    }

    /**
     * Repeatedly tries to connect to an ip and port
     */
    void connectB(const string& ip, const uint16_t port) {
        while(!connectNb(ip, port)) {}
        // while(!(res = connectNb("127.0.0.1", 8080)).has_value()) {}
    }

    /**
     * Sends data in a non-blocking manner
     * @param pData - pointer to data to send
     * @param size  - number of bytes to send
     * @returns number of bytes sent, or -1 for error
     */
    int sendNb(char *pData, size_t size) {
        int res = send(fd_, pData, size, 0);
        if(res != -1) {
            return res;
        } else if(errno == EAGAIN || errno == EWOULDBLOCK){
            return 0;
        } 
        perror("Failed to send data");
        return -1;
    }

    /** 
     * Blocks until all data has been set
     * 
     * @param pData - pointer to data to send
     * @param count - number of bytes to send
    */
    bool sendB(char *pData, size_t target_count) {
        size_t curr_count = 0; // number of bytes that have been sent out

        while(curr_count < target_count) {
            // attempt to send remaining data
            size_t remaining_count = target_count - curr_count;
            int curr_sent = sendNb(pData + curr_count, remaining_count);
            if(curr_sent < 0) {
                std::cerr << "Failed to send data - in sendBlocking" << std::endl;
                return false;
            } 
            curr_count += curr_sent;
        }
        return true;
    }

    /**
    * Returns the number of bytes read from connection
    * @param data - pointer to buffer where data is to be stored
    * @param size - max num bytes which can be copied into buffer
    */
    int receiveNb(char *pData, size_t size) {
        int res = ::recv(fd_, pData, size, 0);
        if(res != -1) {
            return res;
        } else if(errno == EAGAIN || errno == EWOULDBLOCK){
            return 0;
        }
        perror("Failed to receive data");
        return -1;
    }

    /**
     * Block until all data has ben received
     * @param pData - pointer memory where data will be stored
     * @param size - number of bytes to receive
     */
    bool receiveB(char *pData, size_t target_count) {
        size_t curr_count = 0; // number of bytes that have already been received

        while(curr_count < target_count) {
            size_t remaining_count = target_count - curr_count;
            int curr_receive = receiveNb(pData + curr_count, remaining_count);
            if(curr_receive < 0) {
                std::cerr << "Failed to receive data - in receiveBlocking" << std::endl;
                return false;
            }
            curr_count += curr_receive;
        }
        return true;
    }
    
    // destructor closes socket
    ~Socket() {
        close(fd_);
    }
private:
    bool connected_; // whether the socket is connected
    int fd_; // file descriptor on system, representing the socket identifier
};