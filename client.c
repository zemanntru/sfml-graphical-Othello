#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char* argv[])
{
    int sockfd, portno, numbytes;
    struct sockaddr_in serverInfo;
    struct hostent *server;
    
    if(argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(EXIT_FAILURE);
    }
    memset(&serverInfo, 0, sizeof serverInfo);
    serverInfo.sin_family = AF_INET;
    strcpy((char*)server->h_addr, (char*)&serverInfo.sin_addr.s_addr);
    serverInfo.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serverInfo, sizeof serverInfo) == -1){
        perror("ERROR binding");
        exit(EXIT_FAILURE);
    }
    printf("**Connected successfully to the server**\n");
    do
    {
        char buffer[256];
        memset(&buffer, 0, sizeof buffer);
        fgets(buffer, 255, stdin);
        if(!strcmp(buffer, "QUIT\n")){
            close(sockfd);
            exit(EXIT_SUCCESS);
        }

        if((numbytes = send(sockfd, buffer, strlen(buffer), 0)) == -1) {
            perror("ERROR sending");
            exit(EXIT_FAILURE);
        }
        memset(&buffer, 0, sizeof buffer);
        if((numbytes = recv(sockfd, buffer, 255, 0)) == -1) {
            perror("ERROR receiving");
            exit(EXIT_FAILURE);
        }
        printf("%s", buffer);
    } while (1);
    return 0;
}