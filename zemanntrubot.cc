#include "edgetable.hpp"
#include "zemanntrubot.hpp"

MyGameCPU::ZemanntruBot::ZemanntruBot(char color) : mColor(color){}

MyGameCPU::ZemanntruBot::~ZemanntruBot() {}

int128_t MyGameCPU::ZemanntruBot::getAvailableMoves(int128_t player, int128_t opponent)
{   //Doing an O(n) scan for available moves based on all different directions
    int128_t empty = ~ (player | opponent), t, mask, mask2, moves = 0;

    //bitboard move gathering horizontal
    mask = opponent & 0x7E7E7E7E7E7E7E7E;
    for(t = mask & (player << 1), mask2 = mask & (mask << 1); mask2 ; t |= mask2 & (t << 1), mask2 = mask & (mask2 << 1));
    moves |= empty & (t << 1);
    for(t = mask & (player >> 1), mask2 = mask & (mask >> 1); mask2 ; t |= mask2 & (t >> 1), mask2 = mask & (mask2 >> 1));
    moves |= empty & (t >> 1);

    //Bitshift move gathering vertical
    mask = opponent & 0x00FFFFFFFFFFFF00;
    for(t = mask & (player << 8), mask2 = mask & (mask << 8); mask2 ; t |= mask2 & (t << 8), mask2 = mask & (mask2 << 8));
    moves |= empty & (t << 8);
    for(t = mask & (player >> 8), mask2 = mask & (mask >> 8); mask2 ; t |= mask2 & (t >> 8), mask2 = mask & (mask2 >> 8));
    moves |= empty & (t >> 8);
    
    //Bitshift move gathering diagonals
    mask = opponent & 0x007E7E7E7E7E7E00;
    for(t = mask & (player << 9), mask2 = mask & (mask << 9); mask2 ; t |= mask2 & (t << 9), mask2 = mask & (mask2 << 9));
    moves |= empty & (t << 9);
    for(t = mask & (player >> 9), mask2 = mask & (mask >> 9); mask2 ; t |= mask2 & (t >> 9), mask2 = mask & (mask2 >> 9));
    moves |= empty & (t >> 9);
    for(t = mask & (player << 7), mask2 = mask & (mask << 7); mask2 ; t |= mask2 & (t << 7), mask2 = mask & (mask2 << 7));
    moves |= empty & (t << 7);
    for(t = mask & (player >> 7), mask2 = mask & (mask >> 7); mask2 ; t |= mask2 & (t >> 7), mask2 = mask & (mask2 >> 7));
    moves |= empty & (t >> 7);
    
    return moves;
}

int128_t MyGameCPU::ZemanntruBot::getMoveUpdate(int128_t move, int128_t player, int128_t opponent)
{
    int128_t mask, seq, ret = 0;

    //bit reverse gathering horizontal operation
    for(mask = (move >> 1) & 0x7F7F7F7F7F7F7F7F, seq = 0; mask & opponent; seq |= mask, mask = (mask >> 1) & 0x7F7F7F7F7F7F7F7F);  
    if(mask & player) ret |= seq;
    for(mask = (move << 1) & 0xFEFEFEFEFEFEFEFE, seq = 0; mask & opponent; seq |= mask, mask = (mask << 1) & 0xFEFEFEFEFEFEFEFE);
    if(mask & player) ret |= seq;

    //bit reverse gathering vertical operation
    for(mask = (move >> 8) & 0x00FFFFFFFFFFFFFF, seq = 0; mask & opponent; seq |= mask, mask = (mask >> 8) & 0x00FFFFFFFFFFFFFF);
    if(mask & player) ret |= seq;
    for(mask = (move << 8) & 0xFFFFFFFFFFFFFF00, seq = 0; mask & opponent; seq |= mask, mask = (mask << 8) & 0xFFFFFFFFFFFFFF00);
    if(mask & player) ret |= seq;

    //bit reverse gathering  diagonals
    for(mask = (move >> 7) & 0x00FEFEFEFEFEFEFE, seq = 0; mask & opponent; seq |= mask, mask = (mask >> 7) & 0x00FEFEFEFEFEFEFE);
    if(mask & player) ret |= seq;
    for(mask = (move << 7) & 0x7F7F7F7F7F7F7F00, seq = 0; mask & opponent; seq |= mask, mask = (mask << 7) & 0x7F7F7F7F7F7F7F00);
    if(mask & player) ret |= seq;
    for(mask = (move << 9) & 0xFEFEFEFEFEFEFE00, seq = 0; mask & opponent; seq |= mask, mask = (mask << 9) & 0xFEFEFEFEFEFEFE00);
    if(mask & player) ret |= seq;
    for(mask = (move >> 9) & 0x007F7F7F7F7F7F7F, seq = 0; mask & opponent; seq |= mask, mask = (mask >> 9) & 0x007F7F7F7F7F7F7F);
    if(mask & player) ret |= seq;

    return ret; // bitstring with all the indices that require changing
}

int128_t MyGameCPU::ZemanntruBot::edgeStabilityEvaluation(int128_t player, int128_t opponent)
{ //Return pattern based edge configuration evaluations 
    double sum = edgeTable[getTopEdgeKey(player, opponent)] + //A1A8 
        edgeTable[getRightEdgeKey(player, opponent)] + //H1H8
        edgeTable[getBottomEdgeKey(player, opponent)] + //A1H1
        edgeTable[getLeftEdgeKey(player, opponent)];  //A8H8
    return (int128_t)sum;
}

int128_t MyGameCPU::ZemanntruBot::potentialMobilityEvaluation(int128_t player, int128_t opponent)
{   //Count differential empty spaces for non edge pieces of both player and opponent
    int128_t combo = (player | opponent) & 0x007E7E7E7E7E7E00, empty = ~(player | opponent), mask;
    int128_t pplayer = 0, popponent = 0;
    mask = (empty >> 1) & 0xFFFFFFFFFFFFFFFF & combo; 
    while(mask){ 
        int128_t t = mask & -mask;
        if(t & player) {
            pplayer++;
            player ^= t;
        } else if(t & opponent) {
            popponent++; 
            opponent ^= t;
        }
        mask &= mask - 1;
    }

    mask = (empty << 1) & 0xFFFFFFFFFFFFFFFF & combo; 
    while(mask){ 
        int128_t t = mask & -mask;
        if(t & player) {
            pplayer++;
            player ^= t;
        } else if(t & opponent) {
            popponent++; 
            opponent ^= t;
        }
        mask &= mask - 1;
    }

    mask = (empty >> 8) & 0xFFFFFFFFFFFFFFFF & combo; 
    while(mask){ 
        int128_t t = mask & -mask;
        if(t & player) {
            pplayer++;
            player ^= t;
        } else if(t & opponent) {
            popponent++; 
            opponent ^= t;
        }
        mask &= mask - 1;
    }

    mask = (empty << 8) & 0xFFFFFFFFFFFFFFFF & combo; 
    while(mask){ 
        int128_t t = mask & -mask;
        if(t & player) {
            pplayer++;
            player ^= t;
        } else if(t & opponent) {
            popponent++; 
            opponent ^= t;
        }
        mask &= mask - 1;
    }

    mask = (empty >> 7) & 0xFFFFFFFFFFFFFFFF & combo; 
    while(mask){ 
        int128_t t = mask & -mask;
        if(t & player) {
            pplayer++;
            player ^= t;
        } else if(t & opponent) {
            popponent++; 
            opponent ^= t;
        }
        mask &= mask - 1;
    }

    mask = (empty << 7) & 0xFFFFFFFFFFFFFFFF & combo; 
    while(mask){ 
        int128_t t = mask & -mask;
        if(t & player) {
            pplayer++;
            player ^= t;
        } else if(t & opponent) {
            popponent++; 
            opponent ^= t;
        }
        mask &= mask - 1;
    }

    mask = (empty >> 9) & 0xFFFFFFFFFFFFFFFF & combo; 
    while(mask){ 
        int128_t t = mask & -mask;
        if(t & player) {
            pplayer++;
            player ^= t;
        } else if(t & opponent) {
            popponent++; 
            opponent ^= t;
        }
        mask &= mask - 1;
    }

    mask = (empty << 9) & 0xFFFFFFFFFFFFFFFF & combo; 
    while(mask){ 
        int128_t t = mask & -mask;
        if(t & player) {
            pplayer++;
            player ^= t;
        } else if(t & opponent) {
            popponent++; 
            opponent ^= t;
        }
        mask &= mask - 1;
    }
    return 100 * (popponent - pplayer) / (pplayer + popponent + 2);
}

int128_t MyGameCPU::ZemanntruBot::PieceEvaluation(int128_t player, int128_t opponent){
    int128_t playerCnt = __builtin_popcountll(player),
             opponentCnt = __builtin_popcountll(opponent);
    return (playerCnt - opponentCnt) / (playerCnt + opponentCnt + 2);
}

int128_t MyGameCPU::ZemanntruBot::mobilityEvaluation(int128_t player, int128_t opponent)
{    //Count differential move possibilities for both player and opponent
    int128_t mPlayer = __builtin_popcountll(getAvailableMoves(player, opponent)),
            mOpponent = __builtin_popcountll(getAvailableMoves(opponent, player));
    return 100 * (mPlayer - mOpponent) / (mPlayer + mOpponent + 2);
}

int128_t MyGameCPU::ZemanntruBot::evaluateBoard(int128_t player, int128_t opponent)
{   //Dynamic Heurisitc evaluation function
    int128_t sum = 0, stage = __builtin_popcountll(player | opponent);
    int128_t CMOBILITY = 61000 + 7500 * stage,  PMOBILITY = 55000 + 5000 * stage, CEDGE = 2000 + 100 * stage;
    if(stage == 63) CMOBILITY = 1050000;
    sum += CEDGE * edgeStabilityEvaluation(player, opponent) 
    + PMOBILITY *  potentialMobilityEvaluation(player, opponent) + CMOBILITY * mobilityEvaluation(player, opponent);
    return sum;
}

int128_t MyGameCPU::ZemanntruBot::endgameEvaluation(int128_t player, int128_t opponent)
{   //Game winning evaluation
     int128_t playerCnt = __builtin_popcountll(player),
             opponentCnt = __builtin_popcountll(opponent);
    return playerCnt > opponentCnt ? (1LL<<61) : -(1LL<<61);
}

int128_t MyGameCPU::ZemanntruBot::negaMax(int128_t player, int128_t opponent, int depth, int128_t alpha, int128_t beta)
{
    if(depth == 0) 
        return evaluateBoard(player, opponent);
    int128_t moves = getAvailableMoves(player, opponent), best = -(1LL<<62);
    bool turnOpponent = false;
    if(!moves)
    {   //player has no moves 
        turnOpponent = 1; 
        moves = getAvailableMoves(opponent, player);
        if(moves){   
            if(depth == 1) 
                return evaluateBoard(player, opponent);
            //Swap the current alpha and beta values now in opponent's reference
            alpha = -beta; 
            beta = -alpha;
            std::swap(player, opponent);
            --depth;
        } else 
            return endgameEvaluation(player, opponent);
    }
    while(moves) {
        int128_t t = moves & -moves,
                 rev = getMoveUpdate(t, player, opponent);
        best = std::max(best, -negaMax(opponent^rev,player^(rev|t),depth-1,-beta,-alpha));
        alpha = std::max(best, alpha);
        if(alpha > beta) break;
        moves &= moves - 1;
    }
    if(turnOpponent) best = -best;
    return best;
}

int128_t MyGameCPU::ZemanntruBot::negaScout(int128_t player, int128_t opponent, bool bRoot, bool clear, int depth, int128_t alpha, int128_t beta)
{   
    if((((double)clock() - mTimeStart) / CLOCKS_PER_SEC) > TIME_LIMIT)
        return TIMEOUT;

    int128_t a = alpha;
    if(depth == 0) 
        return evaluateBoard(player, opponent);
    else {
        if(clear) mLookupTable.clear();
        else if(mLookupTable.count(player << 64 | opponent) != 0) // an empty flag indicates that this bucket is empty
        {    
            memoEntry lookup = mLookupTable[player << 64 | opponent];
            //Memoize values based on flag values 
            //https://en.wikipedia.org/wiki/Negamax
            if(lookup.mDepth >= depth)
            {
                if(lookup.mFlag == 1) 
                    return lookup.mEval;
                else if(lookup.mFlag == 2) 
                    alpha = std::max(alpha, lookup.mEval);
                else
                    beta = std::min(beta, lookup.mEval);

                if(alpha >= beta) 
                    return lookup.mEval;
            }
        }
    }
    //Get moves for next configuration 
    int128_t moves = getAvailableMoves(player, opponent), bestMove;
    bool turnOpponent = false, init = true;

    if(!moves)
    {   //player has no moves 
        turnOpponent = 1; 
        moves = getAvailableMoves(opponent, player);
        if(moves)
        {   // If opponent has moves
            if(depth == 1) 
            {   //if depth is 1, we cannot skip to the opponent's turn
                //Evaluate and save the entries
                int128_t eval = evaluateBoard(player, opponent);
                mLookupTable[player << 64 | opponent] = {eval, depth, (char)(eval <= alpha ? 3 : (eval >= beta ? 2 : 1))};
                return eval;
            }
            //Swap the current alpha and beta values now in opponent's reference
            alpha = -beta; 
            beta = -alpha;
            std::swap(player, opponent);
            --depth;
        } else {
            //Neither sides having moves implies endgame, and its evaluation is saved
            int128_t eval = endgameEvaluation(player, opponent);
            mLookupTable[player << 64 | opponent] = {eval, depth, (char)(eval <= alpha ? 3 : (eval >= beta ? 2 : 1))};
            return eval;
        }
    }
    
    std::vector<int128_t>sortedMoves = getMoveOrdering(moves, player, opponent);
    bestMove = sortedMoves[0];
    for(const auto &t : sortedMoves)
    {   //Node search is always the LSB at the current moment
        int128_t test = 0,
                 rev = getMoveUpdate(t, player, opponent);
       
        //Null window search
        if(init) 
            test = -negaScout(opponent^rev, player^(rev|t), false, false, depth - 1, -beta, -alpha);
        else {
            test = -negaScout(opponent^rev, player^(rev|t), false, false, depth - 1, -alpha - 1, -alpha);
            
            if(test == TIMEOUT || -test == TIMEOUT)
                return TIMEOUT;
            if(alpha < test && test < beta) 
                test = -negaScout(opponent^rev, player^(rev|t), false, false, depth - 1, -beta, -test);
        }
        
        if(test == TIMEOUT || -test == TIMEOUT)
            return TIMEOUT;
        
        if(test > alpha){
            alpha = test;
            bestMove = t;
        }
        if(alpha >= beta) break; //Alpha Beta pruning
        init = false; 
    } //Save entries after search
    mLookupTable[player << 64 | opponent] = {alpha, depth, (char)(alpha <= a ? 3 : (alpha >= beta ? 2 : 1))};
    if(mLookupTable.size() > (1<<16)) {
        while(mLookupTable.size() > (1<<16))
            mLookupTable.erase(mLookupTable.begin());
    }
    if(turnOpponent) alpha = -alpha; //Flip new lower bound if it is the opponent player
    return bRoot ? bestMove : alpha;
}

std::vector<int128_t> MyGameCPU::ZemanntruBot::getMoveOrdering(int128_t moves, int128_t player, int128_t opponent) {
    int128_t x = moves;
    std::vector<std::pair<int128_t,int128_t>>memo, eval;
    std::vector<int128_t>ord;
    while(x) {
        int128_t t = x & -x, 
                 rev = getMoveUpdate(t, player, opponent);

        memo.push_back({t, __builtin_popcountll(rev)});
        x &= x - 1;
    }
    std::sort(memo.begin(), memo.end(),
         [](const std::pair<int128_t, int128_t>&a, const std::pair<int128_t, int128_t>&b)
          { return a.second < b.second; });

    for(const auto &v: memo) ord.push_back(v.first);
    return ord;
}

int MyGameCPU::ZemanntruBot::getTopEdgeKey(int128_t player, int128_t opponent)
{   // get the edgeTable key based on the top edge configuration
    int128_t x = 0xFF00000000000000;
    int ret = 0;

    if(player & 0x2000000000000) ret += powBase3[0];
    else if(opponent & 0x2000000000000) ret += 2 * powBase3[0];

    for(int ind = 1; x; ++ind){
        int128_t n = x & -x;
        if(n & player) ret += powBase3[ind];
        else if(n & opponent) ret += 2 * powBase3[ind];
        x &= x - 1;
    }
    if(player & 0x40000000000000) ret += powBase3[9];
    else if(opponent & 0x40000000000000) ret += 2 * powBase3[9];
    return ret;
}

int MyGameCPU::ZemanntruBot::getRightEdgeKey(int128_t player, int128_t opponent)
{
    int128_t x = 0x101010101010101;
    int ret = 0;

    if(player & 0x200) ret += powBase3[0];
    else if(opponent & 0x200) ret += 2 * powBase3[0];

    for(int ind = 1; x; ++ind){
        int128_t n = x & -x;
        if(n & player) ret += powBase3[ind];
        else if(n & opponent) ret += 2 * powBase3[ind];
        x &= x - 1;
    }
    if(player & 0x2000000000000) ret += powBase3[9];
    else if(opponent & 0x2000000000000) ret += 2 * powBase3[9];
    return ret;
}


int MyGameCPU::ZemanntruBot::getBottomEdgeKey(int128_t player, int128_t opponent) 
{
    int128_t x = 0xFF;
    int ret = 0;

    if(player & 0x200) ret += powBase3[0];
    else if(opponent & 0x200) ret += 2 * powBase3[0];

    for(int ind = 1; x; ++ind){
        int128_t n = x & -x;
        if(n & player) ret += powBase3[ind];
        else if(n & opponent) ret += 2 * powBase3[ind];
        x &= x - 1;
    }
    if(player & 0x4000) ret += powBase3[9];
    else if(opponent & 0x4000) ret += 2 * powBase3[9];
    return ret;
}

int MyGameCPU::ZemanntruBot::getLeftEdgeKey(int128_t player, int128_t opponent) 
{
    int128_t x = 0x8080808080808080;
    int ret = 0;

    if(player & 0x4000) ret += powBase3[0];
    else if(opponent & 0x4000) ret += 2 * powBase3[0];

    for(int ind = 1; x; ++ind){
        int128_t n = x & -x;
        if(n & player) ret += powBase3[ind];
        else if(n & opponent) ret += 2 * powBase3[ind];
        x &= x - 1;
    }
    if(player & 0x40000000000000) ret += powBase3[9];
    else if(opponent & 0x40000000000000) ret += 2 * powBase3[9];
    return ret;
}


void MyGameCPU::ZemanntruBot::DisplayBitBoard(int128_t black, int128_t white)
{  //Display binary strings in an array configuration
    std::cout << "  "; 
    for(int i = 0; i < BOARD_SIZE; ++i) std::cout << (char)('A' + i) << " ";
    std::cout << std::endl;
    for(int i = 0; i < BOARD_SIZE; ++i){
        std::cout << (char)('1' + i) << " ";
        for(int j = 0; j < BOARD_SIZE; ++j){
            if(white & 1ULL<< ((BOARD_SIZE - i - 1) * BOARD_SIZE + BOARD_SIZE - j - 1)) std::cout << "W ";
            else if(black & 1ULL<< ((BOARD_SIZE - i - 1) * BOARD_SIZE + BOARD_SIZE - j - 1)) std::cout << "B ";
            else std::cout << "_ ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

std::pair<int,int> MyGameCPU::ZemanntruBot::chooseMove(int(&board)[BOARD_SIZE][BOARD_SIZE]) {
    
    int128_t player = 0, opponent = 0, bestMove;
    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            if(board[i][j] == BLACK) 
                player |= (1ULL<<((BOARD_SIZE - 1 - i)*BOARD_SIZE + BOARD_SIZE - 1 - j));
            else if(board[i][j] == WHITE) 
                opponent |= (1ULL<<((BOARD_SIZE - 1 - i)*BOARD_SIZE + BOARD_SIZE - 1 - j));
        } 
    }
    if(mColor == 'W') std::swap(player, opponent);
    int stage = __builtin_popcountll(player|opponent);
    mTimeStart = clock();
    bool bFirst = true;
    int maxDepth = 0;
    for(int depth = std::min(6, 64 - stage); depth <= 64 - stage; ++depth) {
        int128_t move = negaScout(player, opponent, true, bFirst, depth, -(1LL<<50), (1LL<<50)); //Search function
        if(move == TIMEOUT)
            break;
        bestMove = move;
        maxDepth = depth;
        bFirst = false;
    }
    int ret = __builtin_clzll(bestMove);
    int128_t rev = getMoveUpdate(bestMove, player, opponent);
    player ^= (rev | bestMove);
    opponent ^= rev;
    if(mColor == 'W') std::swap(player, opponent);
    
    DisplayBitBoard(player, opponent);

    std::cout << "My bot places at: " << (char)('A' + ret % 8) << (char)('1' + ret / 8) << " in " 
    << std::fixed << std::setprecision(3) << 1000 * (double)(clock() - mTimeStart) / CLOCKS_PER_SEC << "ms" << std::endl;
    std::cout << "==================================" << std::endl << std::endl;
    
    return {ret % 8, ret / 8};
}
