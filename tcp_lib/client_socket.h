#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "socket.h"

using namespace std;
using namespace std::chrono_literals;

class ClientSocket{
public:
    /**
     * Creates a non-blocking IPv4 non-blocking socket
     */
    ClientSocket() : connected_(false) {
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
     */
    optional<int> connectNb(const string& ip, const uint16_t port) {
        sockaddr_in address;
        address.sin_family = AF_INET;
        inet_pton(AF_INET, ip.c_str(), &address.sin_addr.s_addr);
        address.sin_port = htons(port);
        if(connect(fd_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1) {
            // if connection in progress, return
            if((errno == EALREADY) || (errno == EINPROGRESS)) {
                return {}; 
            } 
            // another error occured
            else if(errno != EISCONN) {
                perror("Client failed to connect");
                return {};
            }
        }
        connected_ = true;
        return fd_;
    }

    /**
     * Repeatedly tries to connect to an ip and port
     */
    void connectB(const string& ip, const uint16_t port) {
        optional<int> res;
        while(!(res = connectNb(ip, port)).has_value()) {}
        // while(!(res = connectNb("127.0.0.1", 8080)).has_value()) {}
    }

    /**
     * Sends data in a non-blocking manner
     * @param pData - pointer to data to send
     * @param size  - number of bytes to send
     * @returns number of bytes sent, or -1 for error
     */
    static int sendNb(uint8_t *pData, size_t size) {
        return Socket::sendNb(fd_, pData, size);
    }

    /** 
     * Blocks until all data has been set
     * 
     * @param pData - pointer to data to send
     * @param count - number of bytes to send
    */
    static bool sendB(uint8_t *pData, size_t count) {
        return Socket::sendB(fd_, pData, size);
    }

    /**
    * Returns the number of bytes read from connection
    * @param data - pointer to buffer where data is to be stored
    * @param size - max num bytes which can be copied into buffer
    */
    static int receiveNb(uint8_t *pData, size_t size) {
        return Socket::receiveNb(fd_, pData, size);
    }

    /**
     * Block until all data has ben received
     * @param pData - pointer memory where data will be stored
     * @param size - number of bytes to receive
     */
    static bool receiveB(uint8_t *pData, size_t count) {
        return Socket::receiveB(fd_, pData, count);
    }


    ~ClientSocket() {
        close(fd_);
    }
    
private:
    int fd_; // file descriptor of the client socket
    bool connected_; 
};
