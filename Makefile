CC = g++
CFLAGS = -Wall -std=c++0x  main.cpp bwted.cpp -o bwted

all: main.cpp bwted.cpp
		$(CC) $(CFLAGS)

clean: main.cpp bwted.cpp
		rm -f bwted
