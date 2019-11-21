#makefile

all: shell

shell: shell.cpp
	g++ -g -w -std=c++11 -o shell shell.cpp -lpthread -lrt -lncurses

clean:
	rm shell *.o