#include "board.hpp"

/**
 * @brief Construct a new Graphics:: Board:: Board object
 * This is done using the SFML vertext arrays to get the grid lines and background sufrace
 */

Graphics::Board::Board(){
    grid.setPrimitiveType(sf::Lines);
    grid.resize(2 * (BOARD_SIZE + BOARD_SIZE + 2));
    surface.setPrimitiveType(sf::Quads);
    surface.resize(4);

    // position the four corners of the rectangle using and set them to green
    surface[0].position = sf::Vector2f(0, 0); 
    surface[1].position = sf::Vector2f(BOARD_SIZE * GRID_SIZE, 0);
    surface[2].position = sf::Vector2f(BOARD_SIZE * GRID_SIZE, BOARD_SIZE * GRID_SIZE);
    surface[3].position = sf::Vector2f(0, BOARD_SIZE * GRID_SIZE);
    
    for (int i = 0; i < 4; i++) surface[i].color = sf::Color::Green;
    
    for (int i = 0; i <= BOARD_SIZE; i++)       // draw the vertical lines
    {                                              
        sf::Vertex* ln =  &grid[2*i];
        ln[0].position = sf::Vector2f(0, i * GRID_SIZE);
        ln[1].position = sf::Vector2f(BOARD_SIZE * GRID_SIZE, i * GRID_SIZE);
        ln[0].color = ln[1].color = sf::Color::Black;
    }
    for (int i = 0; i <= BOARD_SIZE; i++)       // draw the horizontal lines
    {
        sf::Vertex* ln =  &grid[2*(i + BOARD_SIZE + 1)];
        ln[0].position = sf::Vector2f(i * GRID_SIZE, 0);
        ln[1].position = sf::Vector2f(i * GRID_SIZE, BOARD_SIZE * GRID_SIZE);
        ln[0].color = ln[1].color = sf::Color::Black;
    }
}

/**
 * @brief 
 * virtual draw function used by sprite classes
 * @param target 
 * @param states 
 */
void Graphics::Board::draw(sf::RenderTarget& target, sf::RenderStates states) const 
{           
    states.transform *= getTransform();                 
    target.draw(surface);
    target.draw(grid);
}

/**
 * @brief Construct a new Board:: Board object
 * 
 * @param color The color of the client
 * @param playerType HUMAN or BOT
 */
Board::Board(sf::Color color, int playerType) : mColor(color), mGameOver(false), mPlayerType(playerType){
    
    // set up the board

    for (int i = 0; i < BOARD_SIZE; i++) 
        for(int j = 0; j < BOARD_SIZE; j++) 
            mBoard[i][j] = EMPTY;
        
    mBoard[3][3] = WHITE;
    mBoard[3][4] = BLACK;
    mBoard[4][3] = BLACK;
    mBoard[4][4] = WHITE;

    if(mColor == sf::Color::Black){
        mBoard[3][2] = POSBLACK;
        mBoard[2][3] = POSBLACK;
        mBoard[4][5] = POSBLACK;
        mBoard[5][4] = POSBLACK;
        mAllowUpdate = true;                // these booleans are set for black since it goes first
        mDone = true;
    } else {
        mBoard[2][4] = POSWHITE;
        mBoard[4][2] = POSWHITE;
        mBoard[3][5] = POSWHITE;
        mBoard[5][3] = POSWHITE;
        mAllowUpdate = false;
        mDone = false;
    }
    assert(mFont.loadFromFile("Lato-Black.ttf")); 
    
    // launch the appropriate thread (interactive vs noninteractive ~ human vs bot)
    if(mPlayerType == HUMAN) 
        mDisplayThread = std::thread{ &Board::RenderInteractiveBoard, this};
    else 
        mDisplayThread = std::thread{ &Board::RenderNonInteractiveBoard, this};
}

/**
 * @brief Destroy the Board:: Board object
 * 
 * Make sure that all the other threads are completed
 * before the object is destroyed
 * 
 * mGameOver is set so the final scores can be displayed
 */

Board::~Board() {
    
    mGameOver = true;
    if(mBotThread.joinable()) mBotThread.join();
    mDisplayThread.join();
}

/**
 * @brief Encode the current state of the board into a 64 char string
 * the mCond variable blocks the this method until the board is updated if mAllowUpdate is true,
 * and returns the subsequent state of the board.
 * @return std::string 
 */
std::string Board::GetEncodedBoard() {
    std::unique_lock<std::mutex> goLock(mMutexGo);
    std::string ret = "";

    if(mAllowUpdate == false) ret += "F";                 // indicates that we don't have any moves on our turn
    else ret += "T";                                      // we do have a move on this turn
    mCond.wait(goLock, [this]{ return !mAllowUpdate; });  // don't bother blocking if we don't have any available moves

    // wait for the bot to finish searching before proceeding
    if(mBotThread.joinable()) mBotThread.join();

    // this lock ensures that no other thread modifies the board while we are reading it
    std::unique_lock<std::mutex> lock(mMutex);
    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            if(mBoard[i][j] == BLACK) ret += "1";
            else if(mBoard[i][j] == WHITE) ret += "2";
            else ret += "0";
        }
    }
    return ret;
}

/**
 * @brief Takes in an encoding of the board and updates the current board
 * 
 * @param src 
 */
void Board::ReceiveUpdate(const std::string& src){
    std::unique_lock<std::mutex> lock(mMutex);
    uint128_t player = 0, 
              opponent = 0;

    // the lock ensures that the board array is not modified by another thread when we are writing new data
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
    lock.unlock(); // we can release the lock since we aren't modifying the board array anymore
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

    if(moves) mAllowUpdate = mDone = true;      // if there are no moves our client cannot have new inputs
    else mAllowUpdate = mDone = false;          
    
    lock.lock();                                // lock the board as we write the available positions in our board array member
    while(moves) {
        int rem = __builtin_clzll(moves & -moves);
        if(mColor == sf::Color::Black)
            mBoard[rem / BOARD_SIZE][rem % BOARD_SIZE] = POSBLACK;
        else
            mBoard[rem / BOARD_SIZE][rem % BOARD_SIZE] = POSWHITE;
        moves &= moves - 1;
    }
}

/**
 * @brief
 * Update the board by flipping the appropriate pieces after a piece has been placed.
 * 
 * @param idx the x coordinate of the board entry
 * @param idy the y coordinate of the board entry
 */

void Board::GetSelfUpdate(int idx, int idy) {

    if (mColor == sf::Color::Black) assert(mBoard[idy][idx] == POSBLACK);
    else assert(mBoard[idy][idx] == POSWHITE);

    // lock the board as we are reading data from the board array member
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
    lock.unlock();
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

    // update the bitboards
    player^=(ret|move);
    opponent^=ret;

    if (mColor == sf::Color::White) 
        std::swap(player, opponent);

    lock.lock();
    // write the new entries into our board array member
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
    std::unique_lock<std::mutex> goLock(mMutexGo);
    mAllowUpdate = false;
    mCond.notify_one();
}

/**
 * @brief Use sfml libraries to render the sprites and the board background
 * 
 * @param window 
 */
void Board::DrawBoard(sf::RenderWindow& window) {
    window.draw(mBackground);
    // this lock ensures the board doesn't change in the middle of rendering the pieces
    std::unique_lock<std::mutex> lock(mMutex);
    int whiteScore = 0, blackScore = 0;
    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            if(mBoard[i][j] == BLACK) {
                sf::CircleShape disc(RADIUS);
                disc.setFillColor(sf::Color::Black);
                disc.setPosition(((float)j+0.5)*GRID_SIZE - RADIUS, ((float)i+0.5)*GRID_SIZE - RADIUS);
                window.draw(disc);
                blackScore++;
            } else if(mBoard[i][j] == WHITE) {
                sf::CircleShape disc(RADIUS);
                disc.setFillColor(sf::Color::White);
                disc.setPosition(((float)j+0.5)*GRID_SIZE - RADIUS, ((float)i+0.5)*GRID_SIZE - RADIUS);
                window.draw(disc);
                whiteScore++;
            } else if(mPlayerType == HUMAN && mAllowUpdate && mBoard[i][j] == POSBLACK) {
                sf::CircleShape disc(RADIUS);
                disc.setFillColor((sf::Color){0,0,0,128});
                disc.setPosition(((float)j+0.5)*GRID_SIZE - RADIUS, ((float)i+0.5)*GRID_SIZE - RADIUS);
                window.draw(disc);
            } else if(mPlayerType == HUMAN && mAllowUpdate && mBoard[i][j] == POSWHITE) {
                sf::CircleShape disc(RADIUS);
                disc.setFillColor((sf::Color){255,255,255,128});
                disc.setPosition(((float)j+0.5)*GRID_SIZE - RADIUS, ((float)i+0.5)*GRID_SIZE - RADIUS);
                window.draw(disc);
            }
        }
    }
    lock.unlock();
    std::string msgWhite = "Count White Tiles: " + std::to_string(whiteScore);
    std::string msgBlack = "Count Black Tiles: " + std::to_string(blackScore);
    sf::Text wText(msgWhite, mFont, 24);
    sf::Text bText(msgBlack, mFont, 24);
    wText.setPosition(LEFT_TXT_OFFSET, WINDOW_HEIGHT - 80);
    bText.setPosition(LEFT_TXT_OFFSET, WINDOW_HEIGHT - 50);
    bText.setFillColor(sf::Color::Black);
    if(mGameOver) {

        // only activated when mGameOver is set in the destructor
        std::string wMsgOverText, bMsgOverText;
        if(whiteScore > blackScore) {
            wMsgOverText = "White wins! (";
            bMsgOverText = "Black Lost! (";
        }
        else if(whiteScore < blackScore) {
            wMsgOverText = "White lost! (";
            bMsgOverText = "Black wins! (";
        }
        else {
            wMsgOverText = "Draw! (";
            bMsgOverText = "Draw! (";
        }
        std::string wsc, bsc;
        std::stringstream sstr;
        sstr << std::fixed << std::setprecision(2) 
        << (double)whiteScore / (whiteScore + blackScore) << ' ' << (double)blackScore / (whiteScore + blackScore);
        sstr >> wsc >> bsc;
        wMsgOverText += wsc + "%)";
        bMsgOverText += bsc + "%)";
        sf::Text wOverText(wMsgOverText, mFont, 24);
        sf::Text bOverText(bMsgOverText, mFont, 24);
        wOverText.setPosition(LEFT_TXT_OFFSET_FINAL, WINDOW_HEIGHT - 80);
        bOverText.setPosition(LEFT_TXT_OFFSET_FINAL, WINDOW_HEIGHT - 50);
        bOverText.setFillColor(sf::Color::Black);
        window.draw(wOverText);
        window.draw(bOverText);
    }
    window.draw(wText);
    window.draw(bText);
}

/**
 * @brief Background GUI thread allows user to click on valid tiles to place the move
 * Launched during the constructor
 */
void Board::RenderInteractiveBoard() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), 
            "Othello Human Client", sf::Style::Titlebar | sf::Style::Close);

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
                    int posx = event.mouseMove.x,               // set the bounds so the cursor tile will always fit on the screen
                        posy = event.mouseMove.y;

                    if (posx >  BOARD_SIZE*GRID_SIZE - RADIUS)
                        posx =  BOARD_SIZE*GRID_SIZE - RADIUS;
                    else if (posx < RADIUS)
                        posx = RADIUS;

                    if (posy > BOARD_SIZE*GRID_SIZE - RADIUS)
                        posy = BOARD_SIZE*GRID_SIZE - RADIUS;
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
                        
                        // only updates when the user clicks on a valid location on their turn
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

/**
 * @brief Launches the background thread for the bot player
 * The only user action this window takes is closing the GUI
 */

void Board::RenderNonInteractiveBoard() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), 
            "Othello Bot Client", sf::Style::Titlebar | sf::Style::Close);
    GameCPU::ZemanntruBot bot(mColor == sf::Color::Black ? 'B' : 'W');
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
                default: break;
            }
        }
        if(mAllowUpdate) {
            if(mDone) {
                mDone = false;
                // launch new thread with the bot search function.
                mBotThread = std::thread([&bot, this]{
                    std::pair<int,int>botMove = bot.chooseMove(mBoard);
                    GetSelfUpdate(botMove.first, botMove.second);
                });
            }
        }
        window.clear(sf::Color(192,192,192));
        DrawBoard(window);
        window.display();
    }
}
