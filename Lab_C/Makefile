all: mypipeline myshell 

myshell:
	gcc -Wall -g myshell.c LineParser.c -o myshell

mypipeline:
	gcc -Wall -g Mypipeline.c -o mypipeline

mem:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./myshell

clean:
	rm -f mypipeline myshell 