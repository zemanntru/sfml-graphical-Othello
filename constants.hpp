#ifndef CONSTANTS_FILE
#define CONSTANTS_FILE

static const int BOARD_SIZE = 8,
          GRID_SIZE = 75,
          WINDOW_WIDTH = 600,
          WINDOW_HEIGHT = 700,
          RADIUS = 36,
          BACKLOG = 5,
          MAXLEN = 128,
          BUF_SIZE = 65, 
          HASH_CONST = 0xFFFF,
          LEFT_TXT_OFFSET = 30,
          LEFT_TXT_OFFSET_FINAL = 350,
          TCP_PORT = 9900;

static const double     SAFETY_FACTOR = 0.98,
                        TIME_LIMIT = 60.0; // our bot has a total of 60s to finish making all of our moves

static const char SETUP_STR[BUF_SIZE + 1] = "00000000000000000000000000002100000012000000000000000000000000000",
                  ENDGAME_STR[BUF_SIZE + 1] = "10000000000000000000000000000000000000000000000000000000000000000";

enum { EMPTY, BLACK, WHITE, POSBLACK, POSWHITE };
enum { HUMAN, BOT, NEWBOT};
#endif