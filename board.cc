#include "board.hpp"

Graphics::Board::Board(){
    grid.setPrimitiveType(sf::Lines);
    grid.resize(2 * (BOARD_SIZE + BOARD_SIZE + 2));
    surface.setPrimitiveType(sf::Quads);
    surface.resize(4);

    surface[0].position = sf::Vector2f(0, 0);
    surface[1].position = sf::Vector2f(BOARD_SIZE * GRID_SIZE, 0);
    surface[2].position = sf::Vector2f(BOARD_SIZE * GRID_SIZE, BOARD_SIZE * GRID_SIZE);
    surface[3].position = sf::Vector2f(0, BOARD_SIZE * GRID_SIZE);
    
    for (int i = 0; i < 4; i++) surface[i].color = sf::Color::Green;
    
    for (int i = 0; i <= BOARD_SIZE; i++){
        sf::Vertex* ln =  &grid[2*i];
        ln[0].position = sf::Vector2f(0, i * GRID_SIZE);
        ln[1].position = sf::Vector2f(BOARD_SIZE * GRID_SIZE, i * GRID_SIZE);
        ln[0].color = ln[1].color = sf::Color::Black;
    }
    for (int i = 0; i <= BOARD_SIZE; i++)
    {
        sf::Vertex* ln =  &grid[2*(i + BOARD_SIZE + 1)];
        ln[0].position = sf::Vector2f(i * GRID_SIZE, 0);
        ln[1].position = sf::Vector2f(i * GRID_SIZE, BOARD_SIZE * GRID_SIZE);
        ln[0].color = ln[1].color = sf::Color::Black;
    }
}

void Graphics::Board::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    target.draw(surface);
    target.draw(grid);
}


Board::Board(sf::Color color) : mColor(color) {
    for (int i = 0; i < BOARD_SIZE; i++) 
        for(int j = 0; j < BOARD_SIZE; j++) 
            mBoard[i][j] = 0;
        
    mBoard[3][3] = WHITE;
    mBoard[3][4] = BLACK;
    mBoard[4][3] = BLACK;
    mBoard[4][4] = WHITE;

    if(mColor == sf::Color::Black){
        mBoard[3][2] = POSBLACK;
        mBoard[2][3] = POSBLACK;
        mBoard[4][5] = POSBLACK;
        mBoard[5][4] = POSBLACK;
        mAllowUpdate = true;
    } else {
        mBoard[2][4] = POSWHITE;
        mBoard[4][2] = POSWHITE;
        mBoard[3][5] = POSWHITE;
        mBoard[5][3] = POSWHITE;
        mAllowUpdate = false;
    }
    mDisplayThread = std::thread{ &Board::RenderBoard, this};
}

Board::~Board() {
    mDisplayThread.join();
}

std::string Board::GetEncodedBoard() {
    std::unique_lock<std::mutex> goLock(mMutexGo);
    mCond.wait(goLock, [&]{ return !mAllowUpdate; });

    std::unique_lock<std::mutex> lock(mMutex);
    std::string ret = "";

    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            if(mBoard[i][j] == BLACK) ret += "1";
            else if(mBoard[i][j] == WHITE) ret += "2";
            else ret += "0";
        }
    }
    return ret;
}

void Board::ReceiveUpdate(const std::string& src){
    std::unique_lock<std::mutex> lock(mMutex);
    uint128_t player = 0,
              opponent = 0;
             
    assert(src.length() == BOARD_SIZE*BOARD_SIZE);
    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            mBoard[i][j] = src[i*BOARD_SIZE + j] - '0';
            if(mBoard[i][j] == BLACK)
                player |= (1ULL<<((BOARD_SIZE - 1 - i)*BOARD_SIZE + BOARD_SIZE - 1 - j));
            else if(mBoard[i][j] == WHITE)
                opponent |= (1ULL<<((BOARD_SIZE - 1 - i)*BOARD_SIZE + BOARD_SIZE - 1 - j));
        }
    }
    if(mColor == sf::Color::White) 
        std::swap(player, opponent);

    uint128_t empty = ~ (player | opponent), t, mask, mask2, moves = 0;
    mask = opponent & 0x7E7E7E7E7E7E7E7E; // bitboard move gathering horizontal 

    for(t = mask & (player << 1), mask2 = mask & (mask << 1); mask2; 
        t |= mask2 & (t << 1), mask2 = mask & (mask2 << 1));
    moves |= empty & (t << 1);

    for(t = mask & (player >> 1), mask2 = mask & (mask >> 1); mask2; 
        t |= mask2 & (t >> 1), mask2 = mask & (mask2 >> 1));
    moves |= empty & (t >> 1);

    mask = opponent & 0x00FFFFFFFFFFFF00; // bitboard move gathering vertical, 

    for(t = mask & (player << 8), mask2 = mask & (mask << 8); mask2; 
        t |= mask2 & (t << 8), mask2 = mask & (mask2 << 8));
    moves |= empty & (t << 8);

    for(t = mask & (player >> 8), mask2 = mask & (mask >> 8); mask2; 
        t |= mask2 & (t >> 8), mask2 = mask & (mask2 >> 8));
    moves |= empty & (t >> 8);
    
    mask = opponent & 0x007E7E7E7E7E7E00; // bitboard move gathering diagonals

    for(t = mask & (player << 9), mask2 = mask & (mask << 9); mask2; 
        t |= mask2 & (t << 9), mask2 = mask & (mask2 << 9));
    moves |= empty & (t << 9);

    for(t = mask & (player >> 9), mask2 = mask & (mask >> 9); mask2; 
        t |= mask2 & (t >> 9), mask2 = mask & (mask2 >> 9));
    moves |= empty & (t >> 9);

    for(t = mask & (player << 7), mask2 = mask & (mask << 7); mask2; 
        t |= mask2 & (t << 7), mask2 = mask & (mask2 << 7));
    moves |= empty & (t << 7);

    for(t = mask & (player >> 7), mask2 = mask & (mask >> 7); mask2; 
        t |= mask2 & (t >> 7), mask2 = mask & (mask2 >> 7));
    moves |= empty & (t >> 7);

    if(moves) mAllowUpdate = true;
    else mAllowUpdate = false;
    
    while(moves) {
        int rem = __builtin_clzll(moves & -moves);
        if(mColor == sf::Color::Black)
            mBoard[rem / BOARD_SIZE][rem % BOARD_SIZE] = POSBLACK;
        else
            mBoard[rem / BOARD_SIZE][rem % BOARD_SIZE] = POSWHITE;
        moves &= moves - 1;
    }
}

void Board::GetSelfUpdate(int idx, int idy) {
    if (mColor == sf::Color::Black) assert(mBoard[idy][idx] == POSBLACK);
    else assert(mBoard[idy][idx] == POSWHITE);
    mAllowUpdate = false;
    std::unique_lock<std::mutex> lock(mMutex);
    uint128_t player = 0,
              opponent = 0,
              move = (1ULL<<((BOARD_SIZE - 1 - idy)*BOARD_SIZE + BOARD_SIZE - 1 - idx)),
              mask, seq, ret = 0;

     for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            if(mBoard[i][j] == BLACK)
                player |= (1ULL<<((BOARD_SIZE - 1 - i)*BOARD_SIZE + BOARD_SIZE - 1 - j));
            else if(mBoard[i][j] == WHITE)
                opponent |= (1ULL<<((BOARD_SIZE - 1 - i)*BOARD_SIZE + BOARD_SIZE - 1 - j));
            mBoard[i][j] = EMPTY;
        }
    }
    if (mColor == sf::Color::White) 
        std::swap(player, opponent);

    //bit reverse gathering horizontal operations
    for(mask = (move >> 1) & 0x7F7F7F7F7F7F7F7F, seq = 0; mask & opponent; 
        seq |= mask, mask = (mask >> 1) & 0x7F7F7F7F7F7F7F7F);  
    if(mask & player) ret |= seq;

    for(mask = (move << 1) & 0xFEFEFEFEFEFEFEFE, seq = 0; mask & opponent; 
        seq |= mask, mask = (mask << 1) & 0xFEFEFEFEFEFEFEFE);
    if(mask & player) ret |= seq;

    //bit reverse gathering vertical operations
    for(mask = (move >> 8) & 0x00FFFFFFFFFFFFFF, seq = 0; mask & opponent; 
        seq |= mask, mask = (mask >> 8) & 0x00FFFFFFFFFFFFFF);
    if(mask & player) ret |= seq;

    for(mask = (move << 8) & 0xFFFFFFFFFFFFFF00, seq = 0; mask & opponent; 
        seq |= mask, mask = (mask << 8) & 0xFFFFFFFFFFFFFF00);
    if(mask & player) ret |= seq;

    //bit reverse gathering diagonal operations
    for(mask = (move >> 7) & 0x00FEFEFEFEFEFEFE, seq = 0; mask & opponent; 
        seq |= mask, mask = (mask >> 7) & 0x00FEFEFEFEFEFEFE);
    if(mask & player) ret |= seq;

    for(mask = (move << 7) & 0x7F7F7F7F7F7F7F00, seq = 0; mask & opponent; 
        seq |= mask, mask = (mask << 7) & 0x7F7F7F7F7F7F7F00);
    if(mask & player) ret |= seq;

    for(mask = (move << 9) & 0xFEFEFEFEFEFEFE00, seq = 0; mask & opponent; 
        seq |= mask, mask = (mask << 9) & 0xFEFEFEFEFEFEFE00);
    if(mask & player) ret |= seq;

    for(mask = (move >> 9) & 0x007F7F7F7F7F7F7F, seq = 0; mask & opponent; 
        seq |= mask, mask = (mask >> 9) & 0x007F7F7F7F7F7F7F);
    if(mask & player) ret |= seq;

    player^=(ret|move);
    opponent^=ret;

    if (mColor == sf::Color::White) 
        std::swap(player, opponent);

    while(player) {
        int rem = __builtin_clzll(player & -player);
        mBoard[rem / BOARD_SIZE][rem % BOARD_SIZE] = BLACK;
        player &= player - 1;
    }
    while(opponent) {
        int rem = __builtin_clzll(opponent & -opponent);
        mBoard[rem / BOARD_SIZE][rem % BOARD_SIZE] = WHITE;
        opponent &= opponent - 1;
    }
    mCond.notify_one();
}

void Board::DrawBoard(sf::RenderWindow& window) {
    window.draw(mBackground);
    std::unique_lock<std::mutex> lock(mMutex);
    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            if(mBoard[i][j] == BLACK) {
                sf::CircleShape disc(RADIUS);
                disc.setFillColor(sf::Color::Black);
                disc.setPosition(((float)j+0.5)*GRID_SIZE - RADIUS, ((float)i+0.5)*GRID_SIZE - RADIUS);
                window.draw(disc);
            } else if(mBoard[i][j] == WHITE) {
                sf::CircleShape disc(RADIUS);
                disc.setFillColor(sf::Color::White);
                disc.setPosition(((float)j+0.5)*GRID_SIZE - RADIUS, ((float)i+0.5)*GRID_SIZE - RADIUS);
                window.draw(disc);
            } else if(mBoard[i][j] == POSBLACK) {
                sf::CircleShape disc(RADIUS);
                disc.setFillColor((sf::Color){0,0,0,128});
                disc.setPosition(((float)j+0.5)*GRID_SIZE - RADIUS, ((float)i+0.5)*GRID_SIZE - RADIUS);
                window.draw(disc);
            } else if(mBoard[i][j] == POSWHITE) {
                sf::CircleShape disc(RADIUS);
                disc.setFillColor((sf::Color){255,255,255,128});
                disc.setPosition(((float)j+0.5)*GRID_SIZE - RADIUS, ((float)i+0.5)*GRID_SIZE - RADIUS);
                window.draw(disc);
            }
        }
    }
}

void Board::RenderBoard() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), 
            "Othello", sf::Style::Titlebar | sf::Style::Close);

    sf::CircleShape cursorTile(RADIUS);
    cursorTile.setFillColor(mColor);

    while(window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            switch (event.type)
            {
                case sf::Event::Closed: { 
                    window.close();
                    break;
                }
                case sf::Event::MouseMoved: {
                    int posx = event.mouseMove.x, 
                        posy = event.mouseMove.y;

                    if (posx >  WINDOW_WIDTH - RADIUS)
                        posx =  WINDOW_WIDTH - RADIUS;
                    else if (posx < RADIUS)
                        posx = RADIUS;

                    if (posy > WINDOW_HEIGHT - RADIUS)
                        posy = WINDOW_HEIGHT - RADIUS;
                    else if (posy < RADIUS)
                        posy = RADIUS;

                    cursorTile.setPosition(posx - RADIUS, posy - RADIUS);
                } break;
                case sf::Event::MouseButtonPressed: {
                    if (mAllowUpdate && event.mouseButton.button == sf::Mouse::Left) {
                        int posx = event.mouseButton.x, 
                            posy = event.mouseButton.y;

                        if (posx >  WINDOW_WIDTH - RADIUS)
                            posx =  WINDOW_WIDTH - RADIUS;
                        else if (posx < RADIUS)
                            posx = RADIUS;
                        if (posy > WINDOW_HEIGHT - RADIUS)
                            posy = WINDOW_HEIGHT - RADIUS;
                        else if (posy < RADIUS)
                            posy = RADIUS;
                        
                        if ((mColor == sf::Color::Black && mBoard[posy/GRID_SIZE][posx/GRID_SIZE] == POSBLACK) || 
                            (mColor == sf::Color::White && mBoard[posy/GRID_SIZE][posx/GRID_SIZE] == POSWHITE)) {
                            GetSelfUpdate(posx/GRID_SIZE, posy/GRID_SIZE);
                        }
                    }
                } break;
                default: break;
            }
        }
        window.clear(sf::Color(192,192,192));
        DrawBoard(window);
        if(mAllowUpdate) window.draw(cursorTile);
        window.display();
    }
}
