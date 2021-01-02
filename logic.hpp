#ifndef LOGIC_FILE
#define LOGIC_FILE

#pragma GCC optimize("O3")
#pragma GCC optimize("unroll-loops")

#include "edgetable.hpp"
#include "constants.hpp"
#include <iostream>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <random>
#include <vector>
#include <limits>
#include <algorithm>
#include <utility>
#include <functional>

typedef __int128 int128_t;

    struct bitset128_128{ 
        int128_t fbits, sbits;
        bool operator==(const bitset128_128& src) const {
            return src.fbits == fbits && src.sbits == sbits;
        }
    };

    struct bitset128_64{ int128_t fbits; 
                         int64_t sbits;};

    struct bitState{ bitset128_64 val; 
                     bitset128_128 bitBoard; 
                     int depth; 
                     char flag; };

namespace GameCPU {

    class ZemanntruBot {
        public:
            ZemanntruBot(char color);
            ~ZemanntruBot();
            ZemanntruBot(const ZemanntruBot& src) = delete;
            ZemanntruBot& operator=(const ZemanntruBot& rhs) = delete;
            void DisplayBitBoard(int128_t player, int128_t opponent);
            std::pair<int,int> chooseMove(int(&board)[BOARD_SIZE][BOARD_SIZE]);
        private:
            int mTimeStart;
            char mColor;
            bitState mLookupTable[1<<16]; // transposition table 
            std::vector<std::vector<uint64_t>>mDistributionTable;
            int128_t getMoveUpdate(int128_t move, int128_t player, int128_t opponent);
            int getTopEdgeKey(int128_t player, int128_t opponent);
            int getRightEdgeKey(int128_t player, int128_t opponent);
            int getBottomEdgeKey(int128_t player, int128_t opponent);
            int getLeftEdgeKey(int128_t player, int128_t opponent);
            int64_t endgameEvaluation(int128_t player, int128_t opponent);
            int64_t edgeStabilityEvaluation(int128_t player, int128_t opponent);
            int64_t potentialMobilityEvaluation(int128_t player, int128_t opponent);
            int64_t mobilityEvaluation(int128_t player, int128_t opponent);
            int64_t evaluateBoard(int128_t player, int128_t opponent, int stage);
            int128_t getAvailableMoves(int128_t player, int128_t opponent);
            int customHash(int128_t player, int128_t opponent);
            bitset128_64 negaScout(int128_t player, int128_t opponent, int depth, int64_t alpha, int64_t beta, int stage);
    };
}
#endif