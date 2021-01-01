#include "board.hpp"
#include <iostream>
#include <string>

int flag = 1;
std::string str;

int main() {
    Board b(sf::Color::Black);
    while(flag) {
        std::cin >> str >> flag;
        b.ReceiveUpdate(str);
        std::cout << b.GetEncodedBoard() << std::endl;
    }
    return 0;
}