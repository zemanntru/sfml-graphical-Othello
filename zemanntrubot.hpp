#ifndef BOTPLAYER_FILE
#define BOTPLAYER_FILE

#pragma GCC optimize("O3")
#pragma GCC optimize("unroll-loops")

#include "constants.hpp"
#include <iostream>
#include <cstdint>
#include <ctime>
#include <bitset>
#include <iomanip>
#include <random>
#include <vector>
#include <limits>
#include <algorithm>
#include <utility>
#include <functional>
#include <string>
#include <cassert>
#include <unordered_map>
#include <iomanip>

typedef __int128 int128_t;

namespace std {
    template <> struct hash<int128_t>
    {
        size_t operator()(int128_t const & key) const 
        {
            string ss = to_string((unsigned long long)(key>>64)) + 
                to_string((unsigned long long)key);
            return hash<string>()(ss);
        }
    };
}


namespace MyGameCPU {
    struct memoEntry{ int128_t mEval;
                      int mDepth;
                      char mFlag; };

    class ZemanntruBot {
        public:
            ZemanntruBot(char color, double timeLimit);
            ~ZemanntruBot();
            ZemanntruBot(const ZemanntruBot& src) = delete;
            ZemanntruBot& operator=(const ZemanntruBot& rhs) = delete;
            std::pair<int,int> chooseMove(int(&board)[BOARD_SIZE][BOARD_SIZE]);
            
        private:
            int mTimeStart;
            char mColor;
            double mMaxTimeAllocation;
            std::unordered_map<int128_t, memoEntry>mLookupTable;
            int128_t getAvailableMoves(int128_t player, int128_t opponent);
            std::vector<int128_t>getMoveOrdering(int128_t moves, int128_t player, int128_t opponent);
            int128_t getMoveUpdate(int128_t move, int128_t player, int128_t opponent);
            int128_t negaMax(int128_t player, int128_t opponent, int depth, int128_t alpha, int128_t beta);
            int128_t negaScout(int128_t player, int128_t opponent, bool bRoot, int depth, int128_t alpha, int128_t beta, std::vector<uint64_t>&numNodes);
            int getTopEdgeKey(int128_t player, int128_t opponent);
            int getRightEdgeKey(int128_t player, int128_t opponent);
            int getBottomEdgeKey(int128_t player, int128_t opponent);
            int getLeftEdgeKey(int128_t player, int128_t opponent);
            int128_t PieceEvaluation(int128_t player, int128_t opponent);
            int128_t endgameEvaluation(int128_t player, int128_t opponent);
            int128_t edgeStabilityEvaluation(int128_t player, int128_t opponent);
            int128_t potentialMobilityEvaluation(int128_t player, int128_t opponent);
            int128_t mobilityEvaluation(int128_t player, int128_t opponent);
            int128_t evaluateBoard(int128_t player, int128_t opponent);
            std::pair<double,double>estimateBranchingFactor(int depth, std::vector<uint64_t>&numNodes);
            int allocateSearchDepth(int128_t player, int128_t opponent);
            int estimateMaximumSearchDepth(int128_t player, int128_t opponent, double timeLimit);
            void DisplayBitBoard(int128_t black, int128_t white);
    };
}
#endif