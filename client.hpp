#include "board.hpp"

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>

class Client {
    public:
        Client(sf::Color);
        Client(const Client& src) = delete;
        Client& operator=(const Client& rhs) = delete;
        void Run();
    private:
        int mSockfd;
        sf::Color mColor;
        struct sockaddr_in mServerInfo;
        struct hostent *mServer;
};