#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>

#define PORT 5461 // port designated for the client apps
#define BACKLOG 5

void sigchld_handler(int s)
{
    // waitpid() might change errno
    int saveErrno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saveErrno;
}

void GetUpdate(int sockfd)
{
    do{
        char buffer[256];
        int numbytes;
        
        if(!strcmp(buffer, "QUIT\n")){
            exit(EXIT_SUCCESS);
        }
        
        memset(&buffer, 0, sizeof buffer);
        if((numbytes = recv(sockfd, buffer, 255, 0)) == -1) {
            perror("ERROR receiving");
            exit(EXIT_FAILURE);
        }
        printf("%s", buffer);

        memset(&buffer, 0, sizeof buffer);
        fgets(buffer, 255, stdin);
        if((numbytes = send(sockfd, buffer, strlen(buffer), 0)) == -1) {
            perror("ERROR sending");
            exit(EXIT_FAILURE);
        }
    } while(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd;
    struct sockaddr_in serverInfo, clientInfo;
    struct sigaction sa;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    memset(&serverInfo, 0, sizeof serverInfo);
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = INADDR_ANY;
    serverInfo.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&serverInfo, sizeof serverInfo) == -1){
        perror("ERROR binding");
        exit(EXIT_FAILURE);
    }

    listen(sockfd, BACKLOG);

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("ERROR sigaction");
        exit(EXIT_FAILURE);
    }

    while(1){
        socklen_t clientLen = sizeof clientInfo;
        newsockfd = accept(sockfd, (struct sockaddr*)&clientInfo, &clientLen);
        if (newsockfd == -1) {
            perror("ERROR accepting");
            exit(EXIT_FAILURE);
        }
        int pid = fork();
        if (pid == -1) {
            perror("ERROR forking");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            close(sockfd); // child need not to be listener
            GetUpdate(newsockfd);
            exit(EXIT_SUCCESS);
        } else 
            close(newsockfd); 
    }
}