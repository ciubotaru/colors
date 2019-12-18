CC = gcc

all: colors

colors: colors.c
	$(CC) -o colors colors.c
