#include <iostream>
#include <algorithm>
#include <iomanip>
#include <functional>
#include <bitset>
#include <cassert>
#include <vector>
#include <unordered_map>

/*
 * Corner, C-square, A-square, B-Square, X-square
 *   C A B B A C
 * C X         X C
 * A             A
 * B             B
 * B             B
 * A             A
 * C x         X C
 *   C A B B A C
 */

enum { UNSTABLE, ALONE, SEMISTABLE, UNANCHORED_STABLE, STABLE1, STABLE2, STABLE3};
const int staticTable[7][4] = {
    { 0, -50, 20, 15 },             // unstable
    { 0, -75, -25, -50},            // alone
    { 0, -125, 100, 100},           // semi-stable
    { 0, 0, 300, 200},              // unanchored stable
    { 0, 800, 800, 800},            // stable-1
    { 0, 1000, 1000, 1000},         // stable-2
    { 800, 1200, 1000, 1000}         // stable-3
};
const int edgeIdx[8] = { 0, 1, 2, 3, 3, 2, 1, 0 };
std::unordered_map<int, double>memodict;
std::unordered_map<int,double> memo;

int getMoveUpdate(int move, int player, int opponent)
{
    int mask, seq, ret = 0;
    //bit reverse gathering horizontal operation
    for(mask = (move >> 1) & 0x7F, seq = 0; mask & opponent; seq |= mask, mask = (mask >> 1) & 0x7F);  
    if(mask & player) ret |= seq;
    for(mask = (move << 1) & 0xFE, seq = 0; mask & opponent; seq |= mask, mask = (mask << 1) & 0xFE);
    if(mask & player) ret |= seq;
    return ret; // bitstring with all the indices that require changing
}

int getAvailableMoves(int player, int opponent) {
    int empty = ~ (player | opponent), 
         t, mask, mask2, moves = 0;
    mask = opponent & 0x7E;
    for(t = mask & (player << 1), mask2 = mask & (mask << 1); mask2 ; t |= mask2 & (t << 1), mask2 = mask & (mask2 << 1));
    moves |= empty & (t << 1);
    for(t = mask & (player >> 1), mask2 = mask & (mask >> 1); mask2 ; t |= mask2 & (t >> 1), mask2 = mask & (mask2 >> 1));
    moves |= empty & (t >> 1);
    return moves;
}

int getUnstableDiscsPlayer(int player, int opponent) {
    std::swap(player, opponent);
    int move = getAvailableMoves(player, opponent),
        rev = getMoveUpdate(move, player, opponent),
        newPlayer = player^(move|rev),
        newOpponent = opponent^rev;
    return opponent & ~newOpponent;
}

int getAloneDiscsPlayer(int player, int opponent) {
    int empty = ~(player | opponent);
    player &= 0x7E;
    return ((empty >> 1) & player) & ((empty << 1) & player);
}

// must occupy A and B squares only
int getUnanchoredDiscsPlayer(int player, int opponent) {
    int unstableOpponent = getUnstableDiscsPlayer(opponent, player);
    if(__builtin_popcount(unstableOpponent) == 2) {
        int mask = (unstableOpponent & (unstableOpponent - 1)) - ((unstableOpponent & -unstableOpponent)<<1);
        if((mask & player) == mask) 
            return mask;
    } 
    return 0;
}

int getStableDiscsTypeOnePlayer(int player, int opponent) {
    int moves = getAvailableMoves(player, opponent),
        typeOne = 0;
    while (moves){
        int t = moves & -moves,
            rev = getMoveUpdate(t, player, opponent),
            newPlayer = player^(t|rev),
            newOpponent = opponent^rev;

        typeOne |= (~(t|rev) & getUnstableDiscsPlayer(newPlayer, newOpponent));
        moves &= moves - 1;
    }
    return typeOne & ~getUnstableDiscsPlayer(player, opponent);
}

int getStableDiscsTypeTwoPlayer(int player, int opponent) {
    // player goes first
    auto getStabilityPlayerFirst = [](int player, int opponent){
        int moves = getAvailableMoves(player, opponent),
            typeTwo = 0;
        while (moves){
            int t = moves & -moves,
                rev = getMoveUpdate(t, player, opponent),
                newPlayer = player^(t|rev),
                newOpponent = opponent^rev,
                newMoves = getAvailableMoves(newOpponent, newPlayer);
                while(newMoves) {
                    int tt = newMoves & -newMoves,
                        rrev = getMoveUpdate(tt, newOpponent, newPlayer),
                        nnewOpponent = newOpponent^(tt|rrev),
                        nnewPlayer = newPlayer^rrev;
                    typeTwo |= (~(t|rev) & ~rrev & getUnstableDiscsPlayer(nnewPlayer, nnewOpponent));
                    newMoves &= newMoves - 1;
                }
            moves &= moves - 1;
        }
        return typeTwo;
    };
    auto getStabilityOpponentFirst = [](int player, int opponent){
        int moves = getAvailableMoves(opponent, player),
            typeTwo = 0;
        while (moves){
            int t = moves & -moves,
                rev = getMoveUpdate(t, opponent, player),
                newPlayer = player^rev,
                newOpponent = opponent^(t|rev),
                newMoves = getAvailableMoves(newPlayer,newOpponent);
                while(newMoves) {
                    int tt = newMoves & -newMoves,
                        rrev = getMoveUpdate(tt, newPlayer, newOpponent),
                        nnewOpponent = newOpponent^rrev,
                        nnewPlayer = newPlayer^(tt|rrev);
                    typeTwo |= (~rev & ~(rrev|tt) & getUnstableDiscsPlayer(nnewPlayer, nnewOpponent));
                    newMoves &= newMoves - 1;
                }
            moves &= moves - 1;
        }
        return typeTwo;
    };
    return (getStabilityPlayerFirst(player, opponent) | 
        getStabilityOpponentFirst(player, opponent)) & ~getUnstableDiscsPlayer(player, opponent);
}

int getStableDiscsTypeThreePlayer(int player, int opponent) {
    std::function<void(int,int,bool)>search;
    int typeThree = player;
    search = [&search, &typeThree](int player, int opponent, bool parity)->void{
         int moves = getAvailableMoves(player, opponent);
         if(!moves) {
             moves = getAvailableMoves(opponent, player);
             if(!moves) {
                if((player | opponent) == 0xFF)
                    return;
                else{
                    int empty = ~(player | opponent);
                    for(int i = 0; i < 7; ++i) {
                        if(empty & (1<<i))
                            search(player, opponent|(1<<i),parity);
                    }
                    return;
                }
             }
             else {
                 parity = !parity;
                 std::swap(player, opponent);
             }
         }
         while(moves) {
             int t = moves & -moves,
                 rev = getMoveUpdate(t, player, opponent),
                 nxtPlayer = opponent^rev,
                 nxtOpponent = player^(rev|t);
            if(parity) typeThree &= nxtOpponent;
            else typeThree &= nxtPlayer;
            search(nxtPlayer, nxtOpponent, !parity);
            moves &= moves - 1;
         }
    };
    search(player, opponent, true);
    search(opponent, player, false);
    return typeThree;
}

int getSemiStable(int player, int opponent) {
    int unstable = getUnstableDiscsPlayer(player, opponent),
        alone = getAloneDiscsPlayer(player, opponent),
        unanchored = getUnanchoredDiscsPlayer(player, opponent),
        stableOne = getStableDiscsTypeOnePlayer(player, opponent),
        stableTwo = getStableDiscsTypeTwoPlayer(player, opponent),
        stableThree = getStableDiscsTypeThreePlayer(player, opponent),
        semiStable = player & ~(unstable | alone | unanchored | stableOne | stableTwo | stableThree);
    return semiStable;
}

void getEvaluationScore(int player, int opponent)
{
    auto evaluate = [](int player, int opponent) -> double {
        int staticIdx[8],
            unstable = getUnstableDiscsPlayer(player, opponent),
            alone = getAloneDiscsPlayer(player, opponent),
            unanchored = getUnanchoredDiscsPlayer(player, opponent),
            stableOne = getStableDiscsTypeOnePlayer(player, opponent),
            stableTwo = getStableDiscsTypeTwoPlayer(player, opponent),
            stableThree = getStableDiscsTypeThreePlayer(player, opponent),
            semiStable = player & ~(unstable | alone | unanchored | stableOne | stableTwo | stableThree);

        std::fill(staticIdx, staticIdx + 8, -1);
        auto getIndices = [&staticIdx](int seq, int label) {
            while(seq) {
                int idx = __builtin_ctz(seq & -seq);
                if(staticIdx[idx] == -1)
                    staticIdx[idx] = label;
                seq &= seq - 1;
            }
        };
        getIndices(unstable, UNSTABLE);
        getIndices(alone, ALONE);
        getIndices(semiStable, SEMISTABLE);
        getIndices(unanchored, UNANCHORED_STABLE);
        getIndices(stableOne, STABLE1);
        getIndices(stableTwo, STABLE2);
        getIndices(stableThree, STABLE3);

        double sum = 0;
        for(int i = 0; i < 8; ++i) {
            if(((1<<i)&player) && (staticIdx[i] != -1)) 
                sum += staticTable[staticIdx[i]][edgeIdx[i]];
        }
        return sum;
    };
    memo[player<<8|opponent] = evaluate(player, opponent) - evaluate(opponent, player);
}

void computeStaticScores(){
    for(int i = 0; i < 6561; ++i) {
        std::string ss;
        for(int num = i; num; num /= 3){
            ss += std::to_string(num % 3);
        }
        int player = 0, opponent = 0;
        for(int j = 0; j < (int)ss.length(); ++j) {
            if(ss[j] == '1')
                player |= (1<<j);
            else if(ss[j] == '2')
                opponent |= (1<<j);
        }
        assert(memo.count(player<<8|opponent) == 0);
        getEvaluationScore(player, opponent);
    }
}

double sampleMinimax(int player, int opponent)
{
    int moves = getAvailableMoves(player, opponent);
    bool turnOpponent = false;
    double best = -10000;
    if(!moves)
    {   //player has no moves 
        turnOpponent = 1; 
        moves = getAvailableMoves(opponent, player);
        if(moves) std::swap(player, opponent);
        else return memo[player<<8|opponent];
        
    }
    while(moves) {
        int t = moves & -moves,
        rev = getMoveUpdate(t, player, opponent);
        best = std::max(best, -sampleMinimax(opponent^rev, player^(rev|t)));
        moves &= moves - 1;
    }
    if(turnOpponent) best = -best;
    return best;
}

// player and opponent are 10 bit numbers
double probabilisticMinimax(int player, int opponent)
{
    if(((player | opponent) & 0x3FF) == 0x3FF) 
    {
        assert(memo.count((((player>>1) & 0xFF)<<8)|((opponent>>1) & 0xFF)));
        return memo[((player>>1) & 0xFF)<<8|((opponent>>1) & 0xFF)];
    }
    // No move is also a legal move
    
    int legalMoves = getAvailableMoves((player>>1) & 0xFF, (opponent>>1) & 0xFF),
        possibleMoves = ~(player | opponent) & ~(legalMoves<<1) & 0x3FF;
    
    std::vector<double>legal;
    std::vector<std::pair<double,double>>possible;

    legal.push_back(-sampleMinimax((opponent>>1) & 0xFF, (player>>1) & 0xFF));
    
    while(legalMoves) {
        int t = legalMoves & -legalMoves,
        rev = getMoveUpdate(t, ((player>>1) & 0xFF), ((opponent>>1) & 0xFF)),
        nxtPlayer =  opponent^(rev<<1), 
        nxtOpponent = player^((rev|t)<<1);
        double val = -probabilisticMinimax(nxtPlayer, nxtOpponent);
        legal.push_back(val);
        legalMoves &= legalMoves - 1;
    }
    int x = possibleMoves;
    while(x) {
        int t = x & -x;
        double coef;
        if(t == 0x1 || t == 0x200) coef = 0.8;
        else if(((t == 0x100) && (opponent & 0x200)) || ((t == 0x2) && (opponent & 0x1))) coef = 0.92;
        else if(((t == 0x100) && (player & 0x200)) || ((t == 0x2) && (player & 0x1))) coef = 0.001;
        else if((t == 0x100) || (t == 0x2)) coef = 0.02;
        else {
            int total = __builtin_popcount(((player>>1)|(opponent>>1))&0xFF) + 2,
                danger = __builtin_popcount(getUnstableDiscsPlayer((opponent>>1) & 0xFF, (player>>1) & 0xFF) |
                                            getAloneDiscsPlayer((opponent>>1) & 0xFF, (player>>1) & 0xFF) |
                                            getSemiStable((opponent>>1) & 0xFF, (player>>1) & 0xFF)) + 1;
            coef = (double)danger / total;
        }
        possible.push_back({-probabilisticMinimax(opponent, player|t), coef});
        x &= x - 1;
    }
    std::sort(legal.begin(), legal.end(), std::greater<double>());
    std::sort(possible.begin(), possible.end(), 
        [](const std::pair<double,double>&a, const std::pair<double,double>&b){
        return a.first > b.first;
    });
    double cutoff = legal[0], sum = 0, acc = 1.0;
    for(auto &u : possible) {
        if(u.first > cutoff) {
            sum += u.first * acc * u.second;
            acc *= ((double)1.0 - u.second);
        }
    }
    sum += acc * legal[0];
    return sum;
}

int main() {
    computeStaticScores();
    std::cout << "double edgeTable[59049] = { ";
    for(int i = 0; i < 59049; ++i){
        std::string ss;
        for(int num = i; num; num /= 3)
            ss += std::to_string(num % 3);
        
        int player = 0, opponent = 0;
        for(int j = 0; j < (int)ss.length(); ++j) {
            if(ss[j] == '1')
                player |= (1<<j);
            else if(ss[j] == '2')
                opponent |= (1<<j);
        }
        std::cout << std::fixed << std::setprecision(3) << probabilisticMinimax(player, opponent) << ", ";
    }
    std::cout << "};";
    return 0;
}
