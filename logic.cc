#include "logic.hpp"

GameCPU::ZemanntruBot::ZemanntruBot(char color) : mDistributionTable(64, std::vector<uint64_t>(3,0)), mColor(color){}

GameCPU::ZemanntruBot::~ZemanntruBot() {}

int128_t GameCPU::ZemanntruBot::getMoveUpdate(int128_t move, int128_t player, int128_t opponent)
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

int GameCPU::ZemanntruBot::getTopEdgeKey(int128_t player, int128_t opponent)
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

int GameCPU::ZemanntruBot::getRightEdgeKey(int128_t player, int128_t opponent)
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


int GameCPU::ZemanntruBot::getBottomEdgeKey(int128_t player, int128_t opponent) 
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

int GameCPU::ZemanntruBot::getLeftEdgeKey(int128_t player, int128_t opponent) 
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


int64_t GameCPU::ZemanntruBot::endgameEvaluation(int128_t player, int128_t opponent)
{   //Game winning evaluation
    return (1LL << 54) * 
    (__builtin_popcountll(player) - __builtin_popcountll(opponent)) / (__builtin_popcountll(player) + __builtin_popcountll(opponent) + 2); 
}

int64_t GameCPU::ZemanntruBot::edgeStabilityEvaluation(int128_t player, int128_t opponent)
{ //Return pattern based edge configuration evaluations 
    return (int64_t)edgeTable[getTopEdgeKey(player, opponent)] + //A1A8 
        edgeTable[getRightEdgeKey(player, opponent)] + //H1H8
        edgeTable[getBottomEdgeKey(player, opponent)] + //A1H1
        edgeTable[getLeftEdgeKey(player, opponent)];  //A8H8
}

int64_t GameCPU::ZemanntruBot::potentialMobilityEvaluation(int128_t player, int128_t opponent)
{   //Count differential empty spaces for non edge pieces of both player and opponent
    int128_t combo = (player | opponent) & 0x007E7E7E7E7E7E00, empty = ~(player | opponent), mask;
    int64_t pplayer = 0, popponent = 0;

    mask = (empty >> 1) & 0xFFFFFFFFFFFFFFFF; mask &= combo; while(mask){ mask & -mask & player ? pplayer++ : popponent++; mask &= mask - 1;}
    mask = (empty << 1) & 0xFFFFFFFFFFFFFFFF; mask &= combo; while(mask){ mask & -mask & player ? pplayer++ : popponent++; mask &= mask - 1;}
    mask = (empty >> 8) & 0xFFFFFFFFFFFFFFFF; mask &= combo; while(mask){ mask & -mask & player ? pplayer++ : popponent++; mask &= mask - 1;}
    mask = (empty << 8) & 0xFFFFFFFFFFFFFFFF; mask &= combo; while(mask){ mask & -mask & player ? pplayer++ : popponent++; mask &= mask - 1;}
    mask = (empty >> 7) & 0xFFFFFFFFFFFFFFFF; mask &= combo; while(mask){ mask & -mask & player ? pplayer++ : popponent++; mask &= mask - 1;}
    mask = (empty << 7) & 0xFFFFFFFFFFFFFFFF; mask &= combo; while(mask){ mask & -mask & player ? pplayer++ : popponent++; mask &= mask - 1;}
    mask = (empty >> 9) & 0xFFFFFFFFFFFFFFFF; mask &= combo; while(mask){ mask & -mask & player ? pplayer++ : popponent++; mask &= mask - 1;}
    mask = (empty << 9) & 0xFFFFFFFFFFFFFFFF; mask &= combo; while(mask){ mask & -mask & player ? pplayer++ : popponent++; mask &= mask - 1;}
    return 100 * (popponent - pplayer) / (pplayer + popponent + 2);
}

int64_t GameCPU::ZemanntruBot::mobilityEvaluation(int128_t player, int128_t opponent)
{    //Count differential move possibilities for both player and opponent
    int64_t mPlayer = __builtin_popcountll(getAvailableMoves(player, opponent)),
            mOpponent = __builtin_popcountll(getAvailableMoves(opponent, player));
    return 100 * (mPlayer - mOpponent) / (mPlayer + mOpponent + 2);
}

int64_t GameCPU::ZemanntruBot::evaluateBoard(int128_t player, int128_t opponent, int stage)
{   //Dynamic Heurisitc evaluation function
    int64_t sum = 0, CMOBILITY = 12000 + stage, PMOBILITY = 2200 + stage;
    if(stage > 61) CMOBILITY = 2987000; //Try to force endgame move
    sum += stage * edgeStabilityEvaluation(player, opponent) + 
    PMOBILITY *  potentialMobilityEvaluation(player, opponent) + CMOBILITY * mobilityEvaluation(player, opponent);
    return sum;
}

int128_t GameCPU::ZemanntruBot::getAvailableMoves(int128_t player, int128_t opponent)
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

int GameCPU::ZemanntruBot::customHash(int128_t player, int128_t opponent)
{   //Zobrist hashing based on values listed in uniform distribution table
    int128_t n = 0xFFFFFFFFFFFFFFFF, x;
    uint64_t key = 0;
    //Produces pseudo-unique keys
    while(n){ 
        x = n & -n; 
        key ^= mDistributionTable[__builtin_ctzll(x)][x & player ? 0 : (x & opponent ? 1 : 2)];
        n &= n - 1;
    }
    return (int)(key & HASH_CONST);
}

bitset128_64 GameCPU::ZemanntruBot::negaScout(int128_t player, int128_t opponent, int depth, int64_t alpha, int64_t beta, int stage)
{   // The main search tree of the program
    if((double)(clock() - mTimeStart) / CLOCKS_PER_SEC > 0.961) return {0, (1LL<<60)};
    int64_t aalpha = alpha, bbeta = beta, eval;
    if(depth == 0) 
        return {0, evaluateBoard(player, opponent, stage)};
    else{
        struct bitState lookup = mLookupTable[customHash(player,opponent)];
        if(lookup.flag != 0) // an empty flag indicates that this bucket is empty
        {    
            if(lookup.bitBoard == (bitset128_128){player,opponent})
            {   //Memoize values based on flag values 
                //https://en.wikipedia.org/wiki/Negamax
                if(lookup.depth >= depth)
                {
                    if(lookup.flag == 1) 
                        return lookup.val;
                    else if(lookup.flag == 2) 
                        aalpha = std::max(aalpha, lookup.val.sbits);
                    else
                        bbeta = std::min(bbeta, lookup.val.sbits);
                    if(aalpha >= bbeta) 
                        return lookup.val;
                }
            }
        }
    }
    //Get moves for next configuration 
    int nxtDepth = depth; 
    int128_t moves = getAvailableMoves(player, opponent), 
             bestMove,
             nxtplayer = opponent, 
             nxtopponent = player;  
    bool turnopponent = 0, init = 1;
    if(!moves)
    {   //player has no moves 
        turnopponent = 1; 
        moves = getAvailableMoves(opponent, player);
        if(moves)
        {   // If opponent has moves
            if(nxtDepth == 1) 
            {   //if depth is 1, we cannot skip to the opponent's turn
                //Evaluate and save the entries
                eval = evaluateBoard(player, opponent, stage);
                mLookupTable[customHash(player,opponent)] = {{0, eval}, {player, opponent}, depth, (char)(eval <= alpha ? 3 : (eval >= beta ? 2 : 1))};
                return {0, eval};
            }
            //Swap the current alpha and beta values now in opponent's reference
            aalpha = -beta; 
            bbeta = -alpha;
            nxtplayer = player; 
            nxtopponent = opponent;
            nxtDepth--;
        } else {
            //Neither sides having moves implies endgame, and its evaluation is saved
            eval = endgameEvaluation(player, opponent);
            mLookupTable[customHash(player,opponent)] = {{0, eval}, {player, opponent}, depth, (char)(eval <= alpha ? 3 : (eval >= beta ? 2 : 1))};
            return {0, eval};
        }
    }
    int64_t a_alpha = aalpha,
            b_beta = bbeta;

    while(moves)
    {   //Node search is always the LSB at the current moment
        int128_t t = moves & -moves,
                 rev = getMoveUpdate(t, nxtopponent, nxtplayer);
        //Null window search
        bitset128_64 cur = negaScout(nxtplayer^rev, nxtopponent^(rev|t), nxtDepth - 1, -b_beta, -a_alpha, stage);
        
        if(cur.sbits == (1LL<<60)) 
            return {0, (1LL<<60)};  //Exit if time limit is reached
        cur.sbits = -cur.sbits; 
        if(!init && (a_alpha < cur.sbits) && (cur.sbits < bbeta))
        {   //Do a full search if Null window search fails
            cur = negaScout(nxtplayer^rev, nxtopponent^(rev|t), nxtDepth - 1, -bbeta, -aalpha, stage);
            if(cur.sbits == (1LL<<60)) 
                return {0, (1LL<<60)}; //Exit if time limit is reached
            cur.sbits = -cur.sbits;
        }
        if(cur.sbits > aalpha){    //Update local maxima
            aalpha = cur.sbits;
            bestMove = t;
        }
        if(aalpha >= bbeta) break; //Alpha Beta pruning
        init = 0; 
        b_beta = aalpha + 1;
        moves &= moves - 1;
    } //Save entries after search
    mLookupTable[customHash(player,opponent)] = {{bestMove, aalpha}, {nxtopponent, nxtplayer}, depth, (char)(aalpha <= alpha ? 3 : (aalpha >= beta ? 2 : 1))};
    if(turnopponent) aalpha = -aalpha; //Flip new lower bound if it is the opponent player
    return {bestMove, aalpha};
}

std::pair<int,int> GameCPU::ZemanntruBot::chooseMove(int(&board)[BOARD_SIZE][BOARD_SIZE]) {
    int128_t player = 0, opponent = 0;

    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            if(board[i][j] == BLACK)
                player |= (1ULL<<((BOARD_SIZE - 1 - i)*BOARD_SIZE + BOARD_SIZE - 1 - j));
            else if(board[i][j] == WHITE)
                opponent |= (1ULL<<((BOARD_SIZE - 1 - i)*BOARD_SIZE + BOARD_SIZE - 1 - j));
        }
    }
    if(mColor == 'W') std::swap(player, opponent);

    int128_t bestMove, combo = player | opponent;
    int stage = __builtin_popcountll(combo); //stage is the number of pieces present on the board

    mTimeStart = clock();

    for(int i = 0; i < (1<<16); ++i) 
        mLookupTable[i] = {{0,0}, {0,0}, 0, 0};
    
    for(auto &v: mDistributionTable){
        std::random_device rndDevice;
        std::mt19937 eng(rndDevice());
        std::uniform_int_distribution<unsigned long long>distr(0,std::numeric_limits<unsigned short int>::max());
        std::generate(std::begin(v), std::end(v), std::bind(distr, eng));
    }

    for(int depth = 1; depth <= 64 - stage; ++depth)
    { //Iterative deepening search
        bitset128_64 res = negaScout(player, opponent, depth, -(1LL<<49), (1LL<<49), stage + depth); //Search function
        if(res.sbits == (1LL<<60)) break; //timeout
        if(mLookupTable[customHash(player, opponent)].bitBoard == (bitset128_128){player,opponent}) // check if entries actually match
            bestMove = mLookupTable[customHash(player, opponent)].val.fbits;
    }
    int ret = __builtin_clzll(bestMove);
    return {ret % 8, ret / 8};
}

                