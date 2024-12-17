#pragma once

#include <iostream>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

void printHostName(int fd) {
    char hostNameBuff[100];
    if(gethostname(hostNameBuff, 100)) {
        perror("Error getting hostname");
        return;
    }
    cout << "Host name: " << hostNameBuff << endl;
}

// void printPeerName(int fd) {
//     sockaddr_in sa;
//     if(getpeername(fd, &sa, sizeof(fd))) {

//     }
// }

// IPv4 = 32 bits
// IPv6 = 128 bits


// struct addrinfo {
//     int ai_flags;
//     int ai_family;  // IPv4 or IPv6. Or 
//     int ai_socktype;
//     int ai_protocol;
//     size_t ai_addrlen;
//     struct sockaddr *ai_addr;
//     char *ai_canonname;
//     struct addrinfo *ai_next;
// }

// // holds socket address information
// struct sockaddr {
//     unsigned short sa_family; // AF_INET (IPv4) or AF_INET6 (IPv6)
//     char sa_data[14]; // 14 bytes of protocol address
// }

// // "derived type of sockaddr"
// struct sockaddr_in {
//     short int sin_family; // socket interent family 
//     unsigned short int sin_port; // port number
//     struct in_addr sin_addr; // internet address
//     unsigned char sin_zero[8]; // padding
// }

// struct in_addr {
//     uint32_t s_addr; // 32 bit
// }

// struct sockaddr_in6 {
//     uint16_t sin6_family;
//     uint16_t sin6_port;
//     uint32_t sin6_flownifo;
//     struct in6_addr sin6_addr; // IPv6 address
//     uint32_t sin6_scope_id;
// }

// struct in6_addr{
//     unsigned char s6_addr[16]; // IPv6 address 128bits
// }