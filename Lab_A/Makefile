# Default target: builds the encoder executable
all: encoder

# Link object files to create the final executable with debugging info
encoder: 
	gcc -Wall -g encoder.c -o encoder

# Debug target for quickly running in debug mode
debug: encoder
	gdb ./encoder

# Clean target to remove generated files
clean:
	rm -f encoder input.txt output.txt
