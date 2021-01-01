LIBS= -lsfml-graphics -lsfml-window -lsfml-system -pthread
CC = g++ -std=c++14

SRCS = main.cc board.cc 

OBJS = $(patsubst %.cc,%.o,$(SRCS))
TARGETS = test

all: $(TARGETS)

%.o: %.cc
	$(CC) -c -g $< -o $@

$(TARGETS): $(OBJS)
	$(CC) $^ -o $@ $(LIBS)

clean:
	@echo "**Removing object files and executable**"
	rm -f *.o all core
