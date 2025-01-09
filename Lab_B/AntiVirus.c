#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <link.h>
#define BUFFER_SIZE 256 // Maximum buffer size for input strings

// Define the structure for a virus
typedef struct virus {
    unsigned short SigSize;  // Signature size (number of bytes in the signature)
    char virusName[16];      // Name of the virus (null-terminated string)
    unsigned char* sig;      // Pointer to the virus signature
} virus;

// Define a linked list structure for storing multiple viruses
typedef struct link {
    struct link *nextVirus;  // Pointer to the next virus in the list
    virus *vir;              // Pointer to the current virus
} link;

// Define a structure for menu options
typedef struct {
    char* name;              // Name of the menu option
    void (*f)(link **virus_list); // Function pointer for the menu option
} fun_desc;


// Function prototypes
/*Task 1a: readVirus and printVirus functions*/
virus* readVirus(FILE* file);
virus* readVirusBigEndian(FILE* file); // Learned from chat GPT
virus* readVirusLittleEndian(FILE* file); // Learned from chat GPT
void printVirus(virus* v, FILE* output);
// End of Task 1

/*Task 1b: list_print, list_append and list_free functions*/
void list_print(link *virus_list, FILE *file);
link* list_append(link *virus_list, virus *v);
void list_free(link *virus_list);
// End of Task 1b

//Functions for the menu
void load_signatures(link **virus_list); // OPTION 1

void print_signatures(link **virus_list); // OPTION 2

void menu_detect_virus(link **virus_list); // OPTION 3
void detect_virus(char *buffer, unsigned int size, link *virus_list);

void fix_file(link **virus_list); // OPTION 4
void neutralize_virus(char *fileName, int signatureOffset);

void quit_program(link **virus_list);// OPTION 5


// Main function that determines the file format and calls the appropriate reader
virus* readVirus(FILE* file) {
    static int is_big_endian = -1; // Static variable to retain state across calls

    // Determine file format on the first call
    if (is_big_endian == -1) {
        char magic[4];
        if (fread(magic, 1, 4, file) < 4) {
            printf("Error reading magic number.\n");
            return NULL;
        }

        if (memcmp(magic, "VIRB", 4) == 0) {
            is_big_endian = 1;
        } else if (memcmp(magic, "VIRL", 4) == 0) {
            is_big_endian = 0;
        } else {
            printf("Invalid magic number. Not a valid signature file.\n");
            return NULL;
        }
    }

    // Call the appropriate function based on the file format
    if (is_big_endian) {
        return readVirusBigEndian(file);
    } else {
        return readVirusLittleEndian(file);
    }
}

// Read a virus from a file in big-endian format
virus* readVirusBigEndian(FILE* file) {
    if (file == NULL) return NULL;

    virus* newVirus = (virus*)malloc(sizeof(virus)); // Allocate memory for the virus
    if (newVirus == NULL) { // Check for allocation failure
        perror("Failed to allocate memory for virus");
        return NULL;
    }

    // Read the signature size in big-endian format
    unsigned char size_bytes[2];
    if (fread(size_bytes, 1, 2, file) < 2) {
        free(newVirus); // Free memory if read fails
        return NULL;
    }
    newVirus->SigSize = (size_bytes[0] << 8) | size_bytes[1];

    // Read the virus name
    if (fread(newVirus->virusName, 1, 16, file) < 16) {
        free(newVirus); // Free memory if read fails
        return NULL;
    }
    newVirus->virusName[15] = '\0'; // Ensure null-termination

    // Allocate memory for the signature
    newVirus->sig = (unsigned char*)malloc(newVirus->SigSize);
    if (newVirus->sig == NULL) {
        perror("Failed to allocate memory for virus signature");
        free(newVirus);
        return NULL;
    }

    // Read the signature
    if (fread(newVirus->sig, 1, newVirus->SigSize, file) < newVirus->SigSize) {
        free(newVirus->sig);
        free(newVirus);
        return NULL;
    }

    return newVirus;
}

// Read a virus from a file in little-endian format
virus* readVirusLittleEndian(FILE* file) {
    if (file == NULL) return NULL;

    virus* newVirus = (virus*)malloc(sizeof(virus)); // Allocate memory for the virus
    if (newVirus == NULL) {
        perror("Failed to allocate memory for virus");
        return NULL;
    }

    // Read the signature size in little-endian format
    unsigned char size_bytes[2];
    if (fread(size_bytes, 1, 2, file) < 2) {
        free(newVirus);
        return NULL;
    }
    newVirus->SigSize = (size_bytes[1] << 8) | size_bytes[0];

    // Read the virus name
    if (fread(newVirus->virusName, 1, 16, file) < 16) {
        free(newVirus);
        return NULL;
    }
    newVirus->virusName[15] = '\0'; // Ensure null-termination

    // Allocate memory for the signature
    newVirus->sig = (unsigned char*)malloc(newVirus->SigSize);
    if (newVirus->sig == NULL) {
        perror("Failed to allocate memory for virus signature");
        free(newVirus);
        return NULL;
    }

    // Read the signature
    if (fread(newVirus->sig, 1, newVirus->SigSize, file) < newVirus->SigSize) {
        free(newVirus->sig);
        free(newVirus);
        return NULL;
    }

    return newVirus;
}

// Print details of a virus to a specified output
void printVirus(virus* v, FILE* output) {
    if (v == NULL || output == NULL) return; // Validate input

    // Print the virus name
    fprintf(output, "Virus name: %s\n", v->virusName);

    // Print the virus signature size
    fprintf(output, "Virus signature size: %d\n", v->SigSize);

    // Print the virus signature in hexadecimal format
    fprintf(output, "Virus signature:\n");
    for (int i = 0; i < v->SigSize; i++) {
        fprintf(output, "%02X ", v->sig[i]);
    }
    fprintf(output, "\n\n"); // Add a blank line for readability
}

// Print all the viruses in the linked list to a file
void list_print(link *virus_list, FILE *file) {
    link *curr = virus_list; // Start from the head of the list
    while (curr != NULL) {   // Traverse the list
        printVirus(curr->vir, file); // Print the virus
        curr = curr->nextVirus; // Move to the next virus
    }
}

// Append a new virus to the end of the linked list
link* list_append(link *virus_list, virus *v) {
    link* new_link = (link*)malloc(sizeof(link)); // Allocate memory for a new link
    if (new_link == NULL) { // Check for allocation failure
        perror("Failed to allocate memory for new link.");
        return virus_list; // Return the original list if allocation fails
    }

    // Initialize the new link
    new_link->vir = v;
    new_link->nextVirus = NULL;

    if (virus_list == NULL) { // If the list is empty
        return new_link; // The new link becomes the head of the list
    }

    link *curr = virus_list; // Start from the head of the list

    // Traverse to the last link
    while (curr->nextVirus != NULL) {
        curr = curr->nextVirus;
    }

    curr->nextVirus = new_link; // Add the new link to the end
    return virus_list; // Return the updated list
}

// Free all the memory used by the linked list
void list_free(link *virus_list) {
    link *curr = virus_list; // Start from the head of the list
    while (curr != NULL) {   // Traverse the list
        link *temp = curr;   // Save the current link
        curr = curr->nextVirus; // Move to the next link

        // Free the memory for the virus and its signature
        free(temp->vir->sig);
        free(temp->vir);
        free(temp); // Free the link itself
    }
}

 // Load virus signatures from a file into a linked list
void load_signatures(link **virus_list) {
   
    if (virus_list == NULL) { // Ensure virus list is initialized
        fprintf(stderr, "Error: virus_list is not initialized.\n");
        return;
    }

    char input[BUFFER_SIZE]; // Buffer to store the file name

    // Prompt the user for the signature file name
    printf("Enter signatures file name: ");
    fgets(input, sizeof(input), stdin); // Read the file name
    input[strcspn(input, "\n")] = '\0'; // Remove trailing newline character

    FILE *file = fopen(input, "rb"); // Open the file in binary mode
    if (file == NULL) { // Check if the file was successfully opened
        perror("Error opening file");
        return;
    }

    // Read all virus signatures from the file
    while (!feof(file)) {
        virus *v = readVirus(file); // Read a single virus
        if (v == NULL) break;      // Stop on read error or EOF
        *virus_list = list_append(*virus_list, v); // Append the virus to the list
    }

    fclose(file); // Close the file
    printf("Signatures loaded successfully.\n");
}

// Print all the loaded virus signatures to the standard output
void print_signatures(link **virus_list) {
    if (virus_list == NULL || *virus_list == NULL) { // Check if the list is empty
        printf("No signatures loaded\n");
        return;
    }
    list_print(*virus_list, stdout); // Print the list to stdout
}

// Prompt the user for a file name, scan the file for viruses, and display results
void menu_detect_virus(link **virus_list) {
    char file_name[BUFFER_SIZE]; // Buffer to store the file name
    printf("Enter the suspected file name: ");
    scanf("%s", file_name); // Read the file name from the user

    FILE *suspected_file = fopen(file_name, "rb"); // Open the file in binary mode
    if (suspected_file == NULL) { // Check if the file was successfully opened
        perror("Failed to open the suspected file.");
        return;
    }

    char buffer[10240]; // 10 KB buffer for file content

    // Read the file into the buffer
    unsigned int bytes_read = fread(buffer, 1, sizeof(buffer), suspected_file);
    fclose(suspected_file); // Close the file

    if (bytes_read == 0) { // Check if any data was read
        fprintf(stderr, "Error: Failed to read any data from the file.\n");
        return;
    }

    detect_virus(buffer, bytes_read, *virus_list); // Detect viruses in the file
}

// Scan a buffer for virus signatures and report detections
void detect_virus(char *buffer, unsigned int size, link *virus_list) {

    link *curr = virus_list; // Start from the head of the virus list

    while (curr != NULL) { // Traverse the virus list

        virus *v = curr->vir; // Get the current virus

        for (int i = 0; i <= size - v->SigSize; i++) { // Loop through the buffer

            // Compare the buffer content with the virus signature
            if (memcmp(&buffer[i], v->sig, v->SigSize) == 0) {
                printf("Virus detected!\n");
                printf("Starting byte: %u\n", i); // Offset of the virus in the file
                printf("Virus name: %s\n", v->virusName);
                printf("Signature size: %u\n\n", v->SigSize);
            }
        }
        curr = curr->nextVirus; // Move to the next virus
    }
}

// Automatically detect and neutralize viruses in a file
void fix_file(link **virus_list) {
    char file_name[BUFFER_SIZE]; // Buffer to store the file name
    printf("Enter the suspected file name: ");
    scanf("%s", file_name); // Read the file name from the user

    FILE *suspected_file = fopen(file_name, "rb"); // Open the file in binary mode
    if (suspected_file == NULL) { // Check if the file was successfully opened
        perror("Error opening suspected file for fixing");
        return;
    }

    char buffer[10240]; // 10 KB buffer for file content
    unsigned int bytes_read = fread(buffer, 1, sizeof(buffer), suspected_file);
    fclose(suspected_file); // Close the file

    if (bytes_read == 0) { // Check if any data was read
        fprintf(stderr, "Error: Failed to read any data from the file.\n");
        return;
    }

    link *curr = *virus_list; // Start from the head of the virus list
    while (curr != NULL) { // Traverse the virus list
        virus *v = curr->vir;
        for (int i = 0; i <= bytes_read - v->SigSize; i++) {
            if (memcmp(&buffer[i], v->sig, v->SigSize) == 0) { // Match found
                printf("Detected virus '%s' at offset %d\n", v->virusName, i);
                neutralize_virus(file_name, i); // Neutralize the virus
            }
        }
        curr = curr->nextVirus; // Move to the next virus
    }
}

// Neutralize a virus in a file by replacing its first byte with a RET instruction
void neutralize_virus(char *fileName, int signatureOffset) {
    FILE *file = fopen(fileName, "rb+"); // Open file for reading and writing
    if (file == NULL) { // Check if the file was successfully opened
        perror("Error opening file for neutralizing");
        return;
    }

    // Move the file pointer to the virus's location
    if (fseek(file, signatureOffset, SEEK_SET) != 0) {
        perror("Error seeking to virus signature location");
        fclose(file);
        return;
    }

    unsigned char retInstruction = 0xC3; // RET instruction in x86
    if (fwrite(&retInstruction, 1, 1, file) != 1) { // Write RET to the file
        perror("Error writing RET instruction to file");
    }

    fclose(file); // Close the file
    printf("Neutralized virus at offset %d in file %s.\n", signatureOffset, fileName);
}

// Free all memory used by the virus list and exit the program
void quit_program(link **virus_list) {
    list_free(*virus_list); // Free the virus list
    *virus_list = NULL;     // Set the list pointer to NULL
    printf("Exiting program. Goodbye!\n");
    exit(0);
}





// Main function to display the menu and execute user commands
int main(int argc, char** argv) {
    link *virus_list = NULL; // Initialize the virus list

    // Define the menu options
    fun_desc menu[] = {
        {"Load signatures", load_signatures},
        {"Print signatures", print_signatures},
        {"Detect viruses", menu_detect_virus},
        {"Fix file", fix_file},
        {"Quit", quit_program},
        {NULL, NULL} // End of the menu
    };

    int size = 5; // Number of menu options

    while (1) { // Main menu loop
        printf("Select operation number from the following menu:\n");
        for (int i = 0; menu[i].name != NULL; i++) { // Print menu options
            printf("%d) %s\n", i + 1, menu[i].name);
        }

        printf("Option: ");
        char input[256];
        if (fgets(input, sizeof(input), stdin) == NULL) break; // Exit on EOF
        int option = atoi(input) - 1; // Convert to zero-based index

        if (option < 0 || menu[option].name == NULL || option + 1 > size) {
            printf("Not within bounds.\n"); // Invalid input
        } else {
            printf("Within bounds.\n");
            menu[option].f(&virus_list); // Execute the selected function
            printf("DONE.\n\n");
        }
    }
}
