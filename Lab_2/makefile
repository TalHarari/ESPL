# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Source files
SRC = myshell.c LineParser.c

# Output executable
TARGET = myshell

# Default target
all: myshell looper mypipe

mypipe:
	gcc -Wall -g mypipe.c -o mypipe

looper:
	gcc -o looper Loopers.c


myshell: myshell.c LineParser.c
	gcc -Wall -g -o myshell LineParser.c myshell.c

# Clean up the build files
clean:
	rm -f myshell looper mypipe
