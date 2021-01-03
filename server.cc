#include "constants.hpp"
#include <arpa/inet.h>
#include <cassert>
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
#include <vector>
#include <queue>

void sigchld_handler(int s)
{
    // waitpid() might change errno
    int saveErrno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saveErrno;
}

int main() {

    struct sockaddr_in serverInfo;
    struct sigaction sa;
    int yes = 1;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Code allows server setup and client connections. To learn more, see
    // https://beej.us/guide/bgnet/html/#client-server-background

    assert(sockfd != -1);
    memset(&serverInfo, 0, sizeof serverInfo);
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = INADDR_ANY;
    serverInfo.sin_port = htons(TCP_PORT);
    
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    assert(bind(sockfd, (struct sockaddr*)&serverInfo, sizeof serverInfo) != -1);

    listen(sockfd, BACKLOG);
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    assert(sigaction(SIGCHLD, &sa, NULL) != -1);
    std::cout << "Waiting for Connections." << std::endl;

    sockaddr_in clientInfo1;
    socklen_t len1 = sizeof clientInfo1;
    int playerOnefd  = accept(sockfd, (struct sockaddr*)&clientInfo1, &len1);
    assert(playerOnefd != -1);

    sockaddr_in clientInfo2;
    socklen_t len2 = sizeof clientInfo2;
    int playerTwofd  = accept(sockfd, (struct sockaddr*)&clientInfo2, &len2);
    assert(playerTwofd != -1);

    std::cout << "Connection to the clients established. Launch game room." << std::endl;
    // The first player to join gets black while the second player is white.
    assert(send(playerOnefd, "B", 1, 0) != -1);
    assert(send(playerTwofd, "W", 1, 0) != -1);

    // Sends a call to black player allow bypassing the initial blocking recv call
    assert(send(playerOnefd, SETUP_STR, BUF_SIZE, 0) != -1);

    int noMoves = 0;
    char buffer[MAXLEN];

    // Finite state machine modelling the player. It starts with a receive call followed by send.
    // The socket file descriptors are swapped each iteration to alternate player turns
    while (true)
    {
        memset(buffer, 0, MAXLEN);
        assert(recv(playerOnefd, &buffer, BUF_SIZE, 0) != -1);
        
        if(buffer[0] == 'F') noMoves++;
        else noMoves = 0;

        buffer[0] = '0';
        assert(send(playerTwofd, &buffer, BUF_SIZE, 0) != -1);
    
        if(noMoves == 2) { // exits the loop when both players do not have a valid move
            std::cout << "game room closed" << std::endl;
            break;
        }
        std::swap(playerOnefd, playerTwofd);
    }

    // notifies the players that the game has finished
    assert(send(playerOnefd, ENDGAME_STR, BUF_SIZE, 0) != -1);
    assert(send(playerTwofd, ENDGAME_STR, BUF_SIZE, 0) != -1);

    close(playerOnefd);
    close(playerTwofd);
    close(sockfd);
}