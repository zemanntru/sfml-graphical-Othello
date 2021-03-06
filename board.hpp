#ifndef BOARD_FILE
#define BOARD_FILE

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <atomic>
#include <cassert>
#include <iostream>
#include <sstream>
#include "constants.hpp"
#include "zemanntrubot.hpp"

typedef unsigned __int128 uint128_t;

namespace Graphics
{
    class Board : public sf::Drawable, public sf::Transformable {
        public:
            Board();
        private:
            virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
            sf::VertexArray surface;
            sf::VertexArray grid;
    };
} 

class Board {
    public:
        Board(sf::Color color, int playerType);
        virtual ~Board();
        Board(const Board& src) = delete;
        Board& operator=(const Board& rhs) = delete;
        void ReceiveUpdate(const std::string &src);
        std::string GetEncodedBoard();
    private:
        void RenderInteractiveBoard();
        void RenderNonInteractiveBoard();
        void Experimental();
        void DrawBoard(sf::RenderWindow& window);
        void GetSelfUpdate(int idx, int idy);
        int mBoard[BOARD_SIZE][BOARD_SIZE];
        int mPlayerType;
        bool mGameOver;
        std::atomic<bool>mAllowUpdate;
        std::atomic<bool>mDone;
        sf::Color mColor;
        sf::Font mFont;
        Graphics::Board mBackground;
        std::condition_variable mCond;
        std::mutex mMutex;                  // this mutex controls access to the board
        std::mutex mMutexGo;                // this mutex controls access to whether the process can output data.
        std::thread mDisplayThread;
        std::thread mBotThread;
};

#endif