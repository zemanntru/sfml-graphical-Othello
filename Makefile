LIBS= -lsfml-graphics -lsfml-window -lsfml-system -pthread
CC = g++ -std=c++17

SRCS = board.cc client.cc logic.cc

OBJS = $(patsubst %.cc,%.o,$(SRCS))
TARGETS = client

all: $(TARGETS)

%.o: %.cc
	$(CC) -c -g $< -o $@

$(TARGETS): $(OBJS)
	$(CC) $^ -o $@ $(LIBS)

clean:
	@echo "**Removing object files and executable**"
	rm -f *.o all core
