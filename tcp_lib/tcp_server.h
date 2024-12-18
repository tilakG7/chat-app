#pragma once

#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "tcp_util.h"
#include "tcp_connection.h"

using namespace std;
using namespace std::chrono_literals;

class ServerSockSet {
public:
    ServerSockSet(const string& ip, const uint16_t port) {
        m_listening_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(m_listening_fd < 0) {
            perror("Error creating a socket m_listening_fd");
        }

        // allow socket to rebind to existing port
        // useful is socket is taking time to tear down
        int opt_val = 1;
        if(setsockopt(m_listening_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val)) < 0) {
            perror("setsockopt failed work");
        }

        // Set socket to non-blocking
        int flags = fcntl(m_listening_fd, F_GETFL, 0);
        if (flags < 0) {
            perror("fcntl failed to get m_listening_fd status flags");
        }
        flags |= O_NONBLOCK;
        if (fcntl(m_listening_fd, F_SETFL, flags) < 0) {
            perror("fcntl failed to set file status flags");
        }

        struct sockaddr_in address; // Socket address structure
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr(ip.c_str());
        address.sin_port = htons(port);

        if(::bind(m_listening_fd, reinterpret_cast<sockaddr*>(&address), sizeof(sockaddr_in)) < 0) {
            perror("failed to bind to given IP");
        }

        if (listen(m_listening_fd, kListenBacklog) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }
    }

    optional<int> getConnection() {
        sockaddr_in address; // address of object we are trying to conenct to
        int addrlen = sizeof(address);
        int res = accept(m_listening_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

        if(res == -1) {
            if((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                return {};
            }
            perror("accept failed");
            return {};
        }
        return res;
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


    ~ServerSockSet() {
        ::close(m_listening_fd);
    }

    void printInfo(){ 
        printHostName(m_listening_fd);
    }

private:
    int m_listening_fd = 0; // file descriptor of the server socket
    static constexpr int kListenBacklog = 5;
};

void printVector(vector<uint8_t> & v) {
    for(auto i : v) {
        cout << i << ", ";
    }
    cout << endl;
}
