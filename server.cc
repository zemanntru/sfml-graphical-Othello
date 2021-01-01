#include "server.hpp"

void Server::sigchld_handler(int s)
{
    // waitpid() might change errno
    int saveErrno = errno;
    while(waitpid(-1, NULL, WHOHANG) > 0);
    errno = saveErrno;
}

Server::Server() {
    mSockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(mSockfd != -1);
    memset(&mServerInfo, 0, sizeof mServerInfo);
    mServerInfo.sin_family = AF_INET;
    mServerInfo.sin_addr.s_addr = INADDR_ANY;
    mServerInfo.sin_port = htons(TCP_PORT);
    int yes = 1;
    setsockopt(mSockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    assert(bind(mSockfd, (struct sockaddr*)&mServerInfo, sizeof mServerInfo) != -1);
    listen(mSockfd, BACKLOG);

}