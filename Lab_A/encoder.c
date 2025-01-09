#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

int debug_mode = 1; // Flag for debug mode
char *encoding_key = "0";
FILE *infile = NULL;
FILE *outfile = NULL;


void set_file(FILE **file, char *arg, char mode) {
    *file = fopen(arg + 2, (mode == 'r') ? "r" : "w");
    if (*file == NULL) {
        fprintf(stderr, "Error: Could not open %s file\n", mode == 'r' ? "input" : "output");
        exit(1);
    }
}

// A function that gets a character from the input, the value of the key needed to add/sub, and a flag 'is_add'.
char encode_character(char c, int key_value, int is_add) {

    // In case of a character
    if (isalpha(c)) { 
        int base = islower(c) ? 'a' : 'A';
        int offset = (c - base + (is_add ? key_value : -key_value) + 26) % 26; // according to the '-' or '+' sign, calculate the offset.
        return base + offset;

    } 
    // In case of a digit
    else if (isdigit(c)) {
        int offset = (c - '0' + (is_add ? key_value : -key_value) + 10) % 10; // according to the '-' or '+' sign, calculate the offset.
        return '0' + offset;
    }
    return c;
}

// A function for extract the arguments and print them according to the debug flag.
void process_arguments(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {

        // Debug mode - off
        if(strcmp(argv[i], "-D") == 0){
            debug_mode = 0;
            continue;
        }
        // Debug mode - on
        else if(strcmp(argv[i], "+D") == 0){
            debug_mode = 1;
            continue;
        }

        else if(debug_mode == 1){
            fprintf(stderr,"%s\n", argv[i]);
        }

        // Extract the encoding key
        if (strncmp(argv[i], "+E", 2) == 0 || strncmp(argv[i], "-E", 2) == 0)
                encoding_key = argv[i];

        //Set the input file if needed.
        if (strncmp(argv[i], "-i", 2) == 0) 
                set_file(&infile, argv[i], 'r');

        //Set the output file if needed.
        if (strncmp(argv[i], "-o", 2) == 0) 
                set_file(&outfile, argv[i], 'w');
    }
}

// A function that iterats the input and encoding it one character a time. 
void encode_text() {
    int key_index = 0;
    int is_add = (encoding_key[0] == '+'); // Define if needed to add or substract
    char *new_encoding_key = encoding_key + 2; // Cut the -/+E from the beginning of the encoding key.
    int key_len = strlen(new_encoding_key); 
    char c;
    FILE *input =  (infile!= NULL) ? infile : stdin; 
    FILE *output = (outfile!= NULL) ? outfile : stdout;
   
    //Main loop, iterate over the input characters. 
    while ((c = fgetc(input)) != EOF) {
        int key_value = new_encoding_key[key_index] - '0'; // Convert the string key into int.
        key_index = (key_index + 1) % key_len;
        fputc(encode_character(c, key_value, is_add), output);
    }

    // If used 'input' or 'output' files 
    if (infile) fclose(infile);
    if (outfile) fclose(outfile);
}

int main(int argc, char *argv[]) {
    process_arguments(argc, argv);
    encode_text();
    return 0;
}
