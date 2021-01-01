#ifndef SERVER_FILE
#define SERVER_FILE

#include "constants.hpp"
#include <arpa/inet.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/wait.h>

class Server {
    public:
        Server();
        Server(const Server& src) = delete;
        Server& operator=(const Server& src) = delete;
        void Run();
        void sigchld_handler(int s);
    private:
        int mSockfd;
        struct sockaddr_in mServerInfo;
        struct sigaction mSa;
};

#endif