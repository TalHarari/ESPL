#include <stdio.h>       // For printf, perror
#include <stdlib.h>      // For exit
#include <string.h>      // For strcmp
#include <sys/types.h>   // For pid_t
#include <sys/wait.h>    // For waitpid
#include <linux/limits.h>// for PATH_MAX
#include <signal.h>
#include "LineParser.h"  // For parseCmdLines and freeCmdLines
#include <unistd.h>
#include <fcntl.h> // For open the files
#include <ctype.h> // For isdigit

// Define statuses for processes
#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0
#define HISTLEN 10 // Maximum number of commands in history.

// Structure to represent a history entry
typedef struct historyEntry {
    char* command;              // The full command line (unparsed)
    struct historyEntry* next;  // Pointer to the next history entry
} historyEntry;

// Global variables for managing history
historyEntry* historyHead = NULL; // Pointer to the start of the history list
historyEntry* historyTail = NULL; // Pointer to the end of the history list
int historySize = 0;             // Current size of the history list


// Process structure to store process information
typedef struct process{
    cmdLine *cmd; //Parsed command line for the process
    pid_t pid;    // Process ID
    int status;   // Status of the process (RUNNING, SUSPENDED, OR TERMINATED)
    struct process *next; // Pointer to the next process int the list.
} process;

//GLobal process list
process *process_list = NULL ; // Head of the linked list for processes.
void removeTerminatedProcesses(process** process_list) {
    process* current = *process_list;
    process* prev = NULL;

    while (current) {
        if (current->status == TERMINATED) {
            process* to_free = current;
            if (prev == NULL) {
                *process_list = current->next;
            } else {
                prev->next = current->next;
            }
            current = current->next;
            freeCmdLines(to_free->cmd);
            free(to_free);
        } else {
            prev = current;
            current = current->next;
        }
    }
}



void addHistory(const char * command){
    // Create a new history entry.
    historyEntry *new_entry = (historyEntry*)malloc(sizeof(historyEntry));
    new_entry->command = strdup(command); // Copy the command string.
    new_entry->next = NULL;

    if(historySize == HISTLEN){ // If the history list is full
        historyEntry *tmp = historyHead; // Remove the oldest entry.
        historyHead = historyHead->next; 
        free(tmp->command); // Free the command string. 
        free(tmp); // Free the structure.
        historySize--; // Update the size of the list.

    }

    //Add new entry to the end of the list
    if(historyTail == NULL){// First entry
        historyHead = historyTail = new_entry;
    }
    else{
        historyTail->next = new_entry;
        historyTail = new_entry;
    }
    historySize++; // Update the size of the list.
}

void printHistory(){
    historyEntry *current = historyHead;
    int index = 1; 
    while( current != NULL){
        printf("%d: %s\n", index,current->command);
        current = current->next;
        index++;
    }
}

const char* getHistoryCommand(int index){
    if (index < 1 || index > historySize) { // Check for out-of-range index
        fprintf(stderr, "Invalid history index: %d\n", index);
        return NULL;
    }

    historyEntry *current = historyHead; 
    for(int i = 1 ; i < index ; i++){ // Go to the desired index
        current = current->next;
    }
    return current->command;
}

void freeHistory(){
    historyEntry *current = historyHead;

    while(current != NULL){
        historyEntry *to_free = current;
        free(to_free->command); // Free the stored command string.
        current = current->next;
        free(to_free); // Free the history entry structure.

    }
    historyHead = historyTail = NULL; // Reset history pointers.
    historySize = 0; // Reset history size.
}

// Updates the status of a process in the process list
void updateProcessStatus(process* process_list, int pid, int status) {

    process* current = process_list;

    while (current != NULL) {

        if (current->pid == pid) { // Find the process with the matching PID
            current->status = status; // Update its status
            return;
        }

        current = current->next; // Move to the next process
    }
}

// Updates the process list by checking the status of each process
void updateProcessList(process** process_list) {
    process* current = *process_list;
    int status;
    while (current != NULL) {

        pid_t result = waitpid(current->pid, &status, WNOHANG | WUNTRACED);

        if (result > 0) {
            if (WIFEXITED(status)) {
                updateProcessStatus(*process_list, current->pid, TERMINATED); // Process has terminated

            } else if (WIFSIGNALED(status)) {
                updateProcessStatus(*process_list, current->pid, TERMINATED); // Process was killed by a signal

            } else if (WIFSTOPPED(status)) {
                updateProcessStatus(*process_list, current->pid, SUSPENDED); // Process is suspended

            } else if (WIFCONTINUED(status)) {
                updateProcessStatus(*process_list, current->pid, RUNNING); // Process has resumed
            }
        }
        current = current->next; // Move to the next process
    }
}



//Helper function to free the process list
void freeProcessList(process* process_list) {
    process* current = process_list;
    while (current != NULL) {
        process* to_free = current;
        freeCmdLines(current->cmd); // Free the cmdLine structure
        current = current->next;
        free(to_free); // Free the process structure itself
    }
}

void addProcess(process **process_list, cmdLine *cmd, pid_t pid) {
    process *new_process = (process *)malloc(sizeof(process));
    if (!new_process) {
        perror("malloc failed");
        exit(1);
    }

    // Create a deep copy of the cmdLine structure
    cmdLine *cmd_copy = parseCmdLines(cmd->arguments[0]);
    if (cmd->inputRedirect) {
        cmd_copy->inputRedirect = strdup(cmd->inputRedirect);
    }
    if (cmd->outputRedirect) {
        cmd_copy->outputRedirect = strdup(cmd->outputRedirect);
    }

    new_process->cmd = cmd_copy;
    new_process->pid = pid;
    new_process->status = RUNNING;
    new_process->next = *process_list;
    *process_list = new_process;
}


void printProcessList(process **process_list) {
    updateProcessList(process_list); // Update statuses

    printf("PID\tCommand\tSTATUS\n");

    process *current = *process_list;
    process *prev = NULL;

    while (current) {
        char *status_str = (current->status == RUNNING) ? "Running" :
                           (current->status == SUSPENDED) ? "Suspended" : "Terminated";

        printf("%d\t%s\t%s\n", current->pid, current->cmd->arguments[0], status_str);

        if (current->status == TERMINATED) {
            process *to_delete = current;
            if (prev == NULL) {
                *process_list = current->next;
            } else {
                prev->next = current->next;
            }

            current = current->next;
            freeCmdLines(to_delete->cmd); // Free the associated cmdLine
            free(to_delete); // Free the process structure
        } else {
            prev = current;
            current = current->next;
        }
    }
}



// Function to display the shell prompt
void displayPrompt() {
    char cwd[PATH_MAX]; // Buffer to hold the current working directory path
    if (getcwd(cwd, sizeof(cwd)) != NULL) { // Get the current working directory
        printf("%s$ ", cwd); // Print the working directory followed by a shell prompt
    } else {
        perror("getcwd failed"); // Display error if getcwd fails
    }
}

// Function to execute a parsed command
void execute(cmdLine *pCmdLine) {
    if (pCmdLine->next != NULL) {
        // Handle pipeline
        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            perror("pipe failed");
            exit(1);
        }

        pid_t pid1 = fork();
        if (pid1 < 0) {
            perror("fork failed");
            exit(1);
        }

        
       
        if (pid1 == 0) { // First child process (left-hand side of the pipe)
            close(STDOUT_FILENO);          // Close standard output
            dup(pipe_fd[1]);               // Redirect standard output to the pipe's write end
            close(pipe_fd[0]);             // Close the read end of the pipe
            close(pipe_fd[1]);             // Close the original write end of the pipe

            if (pCmdLine->inputRedirect) { // Check if the command has input redirection specified
                close(STDIN_FILENO); // Close the standard input file descriptor (FD 0) to prepare for redirection

                // Open the file specified in inputRedirect for reading only (O_RDONLY)
                // The new file descriptor will take the place of FD 0 (STDIN_FILENO) because it was closed
                if (open(pCmdLine->inputRedirect, O_RDONLY) == -1) { 
                    perror("input redirection failed"); // Print an error message if the file cannot be opened
                    _exit(1); // Exit the child process with a failure status
                }
            }

            if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) {
                perror("execvp failed");
                _exit(1);
            }
        }

        pid_t pid2 = fork();
        if (pid2 < 0) {
            perror("fork failed");
            exit(1);
        }

        

        if (pid2 == 0) { // Second child process (right-hand side of the pipe)
            close(STDIN_FILENO);           // Close standard input
            dup(pipe_fd[0]);               // Redirect standard input to the pipe's read end
            close(pipe_fd[1]);             // Close the write end of the pipe
            close(pipe_fd[0]);             // Close the original read end of the pipe

            if (execvp(pCmdLine->next->arguments[0], pCmdLine->next->arguments) == -1) {
                perror("execvp failed");
                _exit(1);
            }
        }

        // Parent process
        close(pipe_fd[0]); // Close read end
        close(pipe_fd[1]); // Close write end
        
        addProcess(&process_list, pCmdLine, pid1); // Add the first process
        addProcess(&process_list, pCmdLine->next, pid2); // Add the second process

        waitpid(pid1, NULL, 0);
        updateProcessStatus(process_list, pid1, TERMINATED);
        waitpid(pid2, NULL, 0);
        updateProcessStatus(process_list, pid2, TERMINATED);
        
        //waitpid(pid1, NULL, 0); // Wait for the first child
        //waitpid(pid2, NULL, 0); // Wait for the second child

    } else { // Handle single command

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            freeCmdLines(pCmdLine);
            exit(1);
        }


        if (pid == 0) { // Child process
            // Input redirection
            if (pCmdLine->inputRedirect) {
                close(STDIN_FILENO);
                if (open(pCmdLine->inputRedirect, O_RDONLY) == -1) {
                    perror("input redirection failed");
                    _exit(1);
                }
            }

            // Output redirection
            if (pCmdLine->outputRedirect) {
                close(STDOUT_FILENO);
                if (open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, 0644) == -1) {
                    perror("output redirection failed");
                    _exit(1);
                }
            }

            if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) {
                perror("execvp failed");
                _exit(1);
            }

        }
        else {// Parent process
            
            addProcess(&process_list, pCmdLine, pid); // Add the process to the process list

            if (pCmdLine->blocking) {
                
                while (1) { // Wait this way so we won't reaped the child process from the system by using 'waitpid'
                    updateProcessList(&process_list); // Dynamically update process statuses
                    process* current = process_list;
                    int found = 0;

                    // Check if the process is still in the list and running
                    while (current != NULL) {
                        if (current->pid == pid && current->status != TERMINATED) {
                            found = 1;
                            break;
                        }
                        current = current->next;
                    }

                    if (!found) break; // Process is terminated, break the loop
                    sleep(1); // Wait briefly to keep the shell responsive
                }
            
            } else {
                fprintf(stderr, "Child process %d running in the background\n", pid);
            }
        }
    }
}


int main() {
    while (1) { // Infinite loop for the shell
         
        displayPrompt(); // Display the shell prompt
        char input[2048]; // Buffer to hold the user's input (max 2048 bytes)

        if (fgets(input, sizeof(input), stdin) == NULL) { // Read input from the user
            perror("fgets failed"); // Print error message if fgets fails
            exit(1); // Exit the program with an error
        }
        
        input[strcspn(input, "\n")] = 0; // Remove the newline character from the input

         // Handle "quit"
        if (strcmp(input, "quit") == 0) { // Check if the user entered "quit"
            freeHistory(); // Free the history list.
            freeProcessList(process_list);
            process_list = NULL;
            break; // Exit the infinite loop
        }

        // Handle "history" command
        if(strcmp(input,"history") == 0){
            printHistory();
            continue;
        }


        //Handle "!!" command
        if(strcmp(input, "!!") == 0){

            if(historySize == 0){ //In case there is no history
                fprintf(stderr, "No commands in history.\n");
                continue;
            }

            const char* last_command = getHistoryCommand(historySize);
            if(last_command != NULL){
                strcpy(input, last_command); // Replace input with last command.
                printf("Executing: %s\n",input); // Debug print.
            }
        }

        //Handle "!n" command
        if(input[0] == '!' && isdigit(input[1])){
            int index = atoi(&input[1]); // Extract the index after '!'.
            const char *command = getHistoryCommand(index);

            if(command != NULL){
                strcpy(input, command); // Replace input with the retrieved command
                printf("Executing: %s\n", input);
            }
        }

        addHistory(input); // Add the command to the history.
        cmdLine *parsedCmd = parseCmdLines(input); // Parse the input into a cmdLine structure

        if (parsedCmd == NULL) { // If parsing failed, continue to the next iteration
            perror("Error in parsing command");
            continue;
        }

       

        // Handle the "cd" command
        if (strcmp(parsedCmd->arguments[0], "cd") == 0) {
            if (parsedCmd->argCount < 2) {
                fprintf(stderr, "cd: missing argument\n");
            } else {
                if (chdir(parsedCmd->arguments[1]) == -1) {//return -1 if couldn't change the directory
                    perror("cd failed"); // Print error if chdir fails
                }
            }

            freeCmdLines(parsedCmd);
            continue;
        }

       
        // Handle "procs" command
        if (strcmp(parsedCmd->arguments[0], "procs") == 0){
            printProcessList(&process_list); // Print the list of processes.
            freeCmdLines(parsedCmd);
            continue; // Skip further execution for this iteration
        }

        
        // Handle "stop <pid>"
        if (strcmp(parsedCmd->arguments[0], "stop") == 0) {
            if (parsedCmd->argCount < 2) {
                fprintf(stderr, "stop: missing process ID\n");
            } else {
                pid_t pid = atoi(parsedCmd->arguments[1]); // Convert argument to PID
                if (kill(pid, SIGSTOP) == -1) {
                    perror("stop failed");
                }
                else{
                    updateProcessStatus(process_list, pid, SUSPENDED); // Update status to SUSPENDED
                }
            }
        
            continue;
        }

        // Handle "wake <pid>"
        if (strcmp(parsedCmd->arguments[0], "wake") == 0) {
            if (parsedCmd->argCount < 2) {
                fprintf(stderr, "wake: missing process ID\n");
            } else {
                pid_t pid = atoi(parsedCmd->arguments[1]); // Convert argument to PID
                if (kill(pid, SIGCONT) == -1) {
                    perror("wake failed");
                }
                else{
                    updateProcessStatus(process_list, pid, RUNNING); // Update status to RUNNING
                    removeTerminatedProcesses(process_list); // Remove all terminated processes
                }
            }
         
            continue;
        }

        // Handle "term <pid>"
        if (strcmp(parsedCmd->arguments[0], "term") == 0) {
            if (parsedCmd->argCount < 2) {
                fprintf(stderr, "term: missing process ID\n");
            } else {
                pid_t pid = atoi(parsedCmd->arguments[1]); // Convert argument to PID
                if (kill(pid, SIGINT) == -1) {
                    perror("term failed");
                }
                else{
                    updateProcessStatus(process_list, pid, TERMINATED); // Update status to TERMINATED
                }
            }
            freeCmdLines(parsedCmd);  // Free the cmdLine structure after use
            continue;
        }

        execute(parsedCmd); // Execute the parsed command
        freeCmdLines(parsedCmd);  // Free the cmdLine structure after use

    }

    return 0; // Return 0 to indicate normal program terminationA
}
