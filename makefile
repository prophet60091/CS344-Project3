default: proj3

proj3: marcel.h marcel.o marcel.c
	gcc -Wall -std=c99 -pedantic-errors  -o proj3 marcel.c

clean:
	rm marcel.o

cleanall: clean
	rm proj3