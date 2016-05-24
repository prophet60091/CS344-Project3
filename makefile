default: proj3

proj3: marcel.h marcel.o marcel.c
	gcc -g -Wall -std=c99 -pedantic-errors  -o smallsh marcel.c

clean:
	rm marcel.o

cleanall: clean
	rm proj3