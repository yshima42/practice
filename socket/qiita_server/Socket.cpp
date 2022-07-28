#include "Socket.hpp"

void Socket::set_listenfd() {
    this->listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->listenfd == -1) {
        std::cout << "socket() failed." << std::endl;
        exit(1);
    }
}

void Socket::set_sockaddr_in() {
    memset(&this->serv_addr, 0, sizeof(this->serv_addr));
}