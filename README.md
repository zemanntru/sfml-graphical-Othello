# SFML-based GUI Othello 

### Background

#### About the Project

A first year Computer Engineering project for Introduction to C programming at the University of Toronto. This code was originally written in C, but refactored into C++ with SFML graphics and some code mechanics reworked to maintain academic integrity. The heuristic and evaluation patterns for edge pieces were inspired by the late 80s Othello engine BILL. Like BILL, this program entails Bayesian probability analysis during the precomputation of lookup tables. 

#### Architecture

This project follows the client-server model using C socket programming libraries. Once the server is launched, it waits until two clients have joined to start the game room. The client that joins first plays black. The server alternates receiving input from one client and sends the updated board to the other client. The server terminates once it has detected that neither clients has an available move. The client has one thread that actively receives data from the server and another thread to display the graphics and run the game logic. 

<center><img src="./sample-game.png" alt="Diagram 1" ></img></center>

#### How to Run this Code

Download and extract the files to a directory of your choice. Run`make` to obtain two executables, `server` and `client`. Run `./server` on the current terminal to launch the server. This must be launched before the clients. Next, open another terminal and run `./client [bot | human]`. This client will play black. Repeat that command on a third terminal, and this client will play white. If the `bot` console argument is selected, the program will automatically update its interface after making the move. If `human` is selected, you must click on one of the semi-transparent tiles indicating valid moves. If there are no valid moves present for a client, their turn will be skipped. The server will terminate once there is no more valid moves on either side. Closing the GUI will terminate the client programs.

### Mechanics 

Some key ideas/features used in the bot logic were:
- **Bitboards** - The positions of player/opponent pieces can be encoded into 128 bit integers. 
- **Alpha-Beta pruning** - speeds up the Nega-Scout minimax search narrowing.
- **Iterative deepening to search** - incremental searches allow states at various depths to be saved in hashtables.
- **Timed searches** - search cutoff times can be easily scaled.
- **Heuristic based evaluations** - Evaluation of potential mobiltiy (whitespace around opponent pieces) and current mobility (number of available moves) as a linear function of stage and search depth. 
- **Pattern based evaluations** - A precomputed lookup table is used to analyze edge pieces and adjacent corner pieces based on Bayesian probability and future stability of a player's pieces.
- **Customized hashtable** - transposition table to store memoize board states. 
- **Zobrist hashing** - allows effective encoding of the entire board into a 128 bit number. 
- **Mersenne twister PRNG** - to generate better quality pseudorandom hashes. 

### Ideas/features for further improvement
- **Opening book moves** - increases the chance of winning by using template moves to establish strong beginning/midgame positions.
- **Parity** - Placing your piece in pockets with an odd number of squares in the endgame greatly increases chances of winning.
- **More pattern based evaluations** - Pattern based precomputations can be applied to all diagonals, rows and columns.
- **Move ordering** - Ordering the search to start with subtrees that are more likely to yield effective results. 
- **Monte Carlo Search** - Gaining greater search depth with the risk of missing a critical move. 

### Citation:
**Implementing the Othello bot logic** - Lee, Kai-Fu and Sanjoy Mahajan. “BILL : a table-based, knowledge-intensive othello program.” (1986). https://www.semanticscholar.org/paper/BILL-%3A-a-table-based%2C-knowledge-intensive-othello-Lee-Mahajan/f7ab15736ecc79452aa4546cf8d7f5aa94d6afa0

**Implementing the client-server model** - https://beej.us/guide/bgnet/html/#client-server-background
