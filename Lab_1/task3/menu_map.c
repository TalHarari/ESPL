#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../task2/base.h"
#define BUFFER_SIZE 256

// learned from https://www.programiz.com/c-programming/c-structures
typedef struct{
    char* name;
    char (*f)(char); 
} fun_desc;
///////////////


/////////////Task 2 /////////////
char my_get(char c){
  return fgetc(stdin);
}

char cprt(char c){
  if(c >= 0x20 && c <= 0x7E){
    printf("%c\n",c);
  }
  else{
    printf(".\n");
  }
  return c ;
}

char encrypt(char c) {
    if (c >= 0x1F && c <= 0x7E) {
        return c + 1; // Encrypt by adding 1
    }
    return c; // Return unchanged if outside range
}

char decrypt(char c) {
    if (c >= 0x21 && c <= 0x7F) {
        return c - 1; // Decrypt by subtracting 1
    }
    return c; // Return unchanged if outside range
}

char xprt(char c){
  if(c < 0x20 || c > 0x7E){
    printf(".\n");
  }
    
  else{
    printf("%X\n",c); //Print hexadecimal value
  }
    
  return c; 
}

char dprt (char c){
  if(c < 0x20 || c > 0x7E){
    printf(".\n");
  }
    
  else{
    printf("%d\n", c); // Print decimal value
  }
    
  return c; 
}



char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char))); // Provided 
  
  /* TODO: Complete during task 2.a */
  if (mapped_array == NULL) {
        perror("Failed to allocate memory");
        exit(1);
    }
    
     for (int i = 0; i < array_length; i++) {
        mapped_array[i] = f(array[i]); // Apply function f to each element
    }

  return mapped_array;
}
/////////////Task 2 /////////////



int main(){
    char input[BUFFER_SIZE];

    // chatGPT//
    char* carray = malloc(5 * sizeof(char));
    if (!carray) exit(1);
    memset(carray, '\0', 5); // make it empty string.
    // chatGPT//



    int option;
    fun_desc menu[] = {{"Get String (my_get)", my_get}, {"Print Character (cprt)", cprt},
    {"Encrypt",encrypt},{"Decrypt",decrypt},{"Print Hexadecimal (xprt)",xprt},
    {"Print Decimal (dprt)",dprt},{NULL,NULL}};
    int size;
    for(size = 0; menu[size].name != NULL ; size++){} // count length of the array
        


    //main loop
    while(1){

        // Display the menu
        printf("Select operation number from the following menu: (ctrl^D for exit)\n");
        for(int i = 0; menu[i].name != NULL ; i++){
            printf("%d) %s\n", i , menu[i].name);
        }

        printf("Option: ");

        //chatGPT//
        // If found EOF (fgets return NULL) 
        if (fgets(input, sizeof(input), stdin) == NULL)
            break;
        printf("\n");
        option = atoi(input); // extract the option as int. 
        //chatGPT//


        // check if entered valid option
        if (option < 0 || menu[option].name == NULL || option > size) {
            printf("Not within bounds.\n");
            exit(1);
        } 
        else {
            printf("Within bounds.\n");
        }

        char* new_carray = map(carray,5,menu[option].f); // String with length 5 allocated on the heap. 
        free(carray);
        carray = new_carray;
        printf("DONE.\n\n");
        
    }
free(carray);
return 0;
}