CC = gcc

all: colors

colors: colors.c
	$(CC) -Wall -o colors colors.c -lncurses
