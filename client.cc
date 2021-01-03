#include "board.hpp"

#include <cassert>
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

int main(int argc, char *argv[]) {

    if(argc != 2) {
        fprintf(stderr, "usage %s (human | bot)\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd, playerType;
    sf::Color mColor;
    struct sockaddr_in serverInfo;
    struct hostent *server;
    bool go = 1;
    char buffer[MAXLEN];

    // Code allows server connection. To learn more, see
    // https://beej.us/guide/bgnet/html/#client-server-background

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd != -1);
    server = gethostbyname("localhost");
    assert(server != nullptr);
    memset(&serverInfo, 0, sizeof serverInfo);
    serverInfo.sin_family = AF_INET;
    strcpy((char*)server->h_addr, (char*)&serverInfo.sin_addr.s_addr);
    serverInfo.sin_port = htons(TCP_PORT);
    assert(connect(sockfd, (struct sockaddr *)&serverInfo, sizeof serverInfo) != -1);
    memset(buffer, 0, MAXLEN);
    assert(recv(sockfd, &buffer, 1, 0) != -1);
   
    if(!strcmp("human", argv[1]))
        playerType = HUMAN;
    else if(!strcmp("bot", argv[1]))
        playerType = BOT;
    else
        assert(false);
    std::cout << "Established connection to server" << std::endl;
    Board myBoard((buffer[0]=='B' ? sf::Color::Black : sf::Color::White), playerType);

    // The client model alternates between receiving a call and sending out a call
    while (go) {
        memset(buffer, 0, MAXLEN);
        assert(recv(sockfd, &buffer, BUF_SIZE, 0) != -1); 
        
        // first bit game flag, remaining 64 bits the compressed board
        if (buffer[0] == '0') {
            std::string src(buffer);
            src = src.substr(1,64);
            myBoard.ReceiveUpdate(src);
            src = myBoard.GetEncodedBoard();
            memset(buffer, 0, MAXLEN);
            strcpy(buffer, src.c_str());
            assert(send(sockfd, buffer, BUF_SIZE, 0) != -1);
        } else if (buffer[0] == '1') 
            break;
    }
    std::cout << "game finished" << std::endl;
}