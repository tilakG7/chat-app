#pragma once

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
    Socket() = delete;

    
    /**
     * Sends data in a non-blocking manner
     * @param fd - file descriptor representing connected socket
     * @param pData - pointer to data to send
     * @param size  - number of bytes to send
     * @returns number of bytes sent, or -1 for error
     */
    static int sendNb(int fd, int8_t *pData, size_t size) {
        int res = send(fd, pData, size, 0);
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
    static bool sendB(int fd, uint8_t *pData, size_t count) {
        size_t curr_count = 0; // number of bytes that have been sent out

        while(curr_count < count) {
            // attempt to send remaining data
            int curr_sent = sendNb(fd, pData + curr_count, count - curr_count);
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
    static int Socket::receiveNb(int fd, uint8_t *data, size_t size) {
        int res = ::recv(fd, data, size, 0);
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
    static bool Socket::receiveB(int fd, uint8_t *pData, size_t count) {
        size_t curr_count = 0; // number of bytes that have already been received

        while(curr_count < count) {
            int curr_receive = receiveNb(fd, pData + curr_count, count - curr_count);
            if(curr_receive < 0) {
                std::cerr << "Failed to receive data - in receiveBlocking" << std::endl;
                return false;
            }
            curr_count += curr_receive;
        }
        return true;
    }
};