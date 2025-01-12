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
#include "socket.h"

using namespace std;
using namespace std::chrono_literals;

class ServerSocket {
public:
    ServerSocket(const string& ip, const uint16_t port) {
        fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if(fd_ < 0) {
            perror("Error creating a socket fd_");
        }

        // allow socket to rebind to existing port
        // useful if socket is taking time to tear down
        int opt_val = 1;
        if(setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val)) < 0) {
            perror("setsockopt failed work");
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

        struct sockaddr_in address; // Socket address structure
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr(ip.c_str());
        address.sin_port = htons(port);

        if(::bind(fd_, reinterpret_cast<sockaddr*>(&address), sizeof(sockaddr_in)) < 0) {
            perror("failed to bind to given IP");
        }

        if (listen(fd_, kListenBacklog) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }
    }

    /**
     * Get connection returns an active TCP/IP connection. 
     * The listening port will listen for connections and will connect to the 
     */
    bool getConnection(int &res) {
        sockaddr_in address; // address of object we are trying to conenct to
        int addrlen = sizeof(address);
        res = accept(fd_, (struct sockaddr *)&address, (socklen_t*)&addrlen);

        if(res == -1) {
            if((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                return false;
            }
            perror("accept failed");
            return false;
        }
        return true;
    }


    ~ServerSocket() {
        ::close(fd_);
    }

    void printInfo(){ 
        printHostName(fd_);
    }

private:
    int fd_ = 0; // file descriptor of the server socket
    static constexpr int kListenBacklog = 5;
};