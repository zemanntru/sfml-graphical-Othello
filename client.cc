#ifndef CLIENT_FILE
#define CLIENT_FILE

#include "client.hpp"

Client::Client(sf::Color color) : mColor(color) {
    mSockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(mSockfd != -1);
    mServer = gethostbyname("localhost");
    assert(mServer != nullptr);

    memset(&mServerInfo, 0, sizeof mServerInfo);
    mServerInfo.sin_family = AF_INET;
    strcpy((char*)mServer->h_addr, (char*)&mServerInfo.sin_addr.s_addr);
    mServerInfo.sin_port = htons(TCP_PORT);

    assert(connect(mSockfd, (struct sockaddr *)&mServerInfo, sizeof mServerInfo) != -1);

    std::cout << "Established connection with the server." << std::endl;
}

void Client::Run() {
    Board myBoard(mColor);
    bool go = 1;
    while (go) {
        const char* packet = nullptr;
        assert(recv(mSockfd, &packet, sizeof packet, 0) != -1);
        assert(strlen(packet) == 65); // first bit game flag, remaining 64 bits the compressed board
        if (packet[0] == '0') {
            std::string src(++packet);
            assert(src.length() == 64);
            myBoard.ReceiveUpdate(src);
            src = myBoard.GetEncodedBoard();
            packet = src.c_str();
            assert(send(mSockfd, packet, sizeof packet, 0) != -1);
        } else if (packet[0] == '1') 
            break;
    }
}

#endif