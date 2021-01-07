LIBS= -lsfml-graphics -lsfml-window -lsfml-system -pthread
CC = g++ -std=c++17

CLIENT_SRCS = board.cc client.cc zemanntrubot.cc 
SERVER_SRCS = server.cc

OBJS_CLIENT = $(patsubst %.cc,%.o,$(CLIENT_SRCS))
OBJS_SERVER = $(patsubst %.cc,%.o,$(SERVER_SRCS))

%.o: %.cc
	$(CC) -c -g $< -o $@

all: client server

client: $(OBJS_CLIENT)
	$(CC) $^ -o $@ $(LIBS)
    
server: $(OBJS_SERVER)
	$(CC) $^ -o $@ 

clean:
	@echo "**Removing object files and executable**"
	rm -f *.o all core
