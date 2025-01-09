// learned about 'dup2' function from: https://www.geeksforgeeks.org/dup-dup2-linux-system-call/

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
    pid_t pid = fork(); // Create a child process

    if (pid < 0) { // Check if fork failed
        perror("fork failed"); // Print error message
        exit(1); // Exit the program with an error
    }

    if (pid == 0) { // Child process]

/* Task 3: Add standart input and output redirections capabilities */

//input redirect
if (pCmdLine->inputRedirect) {
    // Close STDIN_FILENO (file descriptor 0).
    close(STDIN_FILENO);

    // Open the file again, reusing the lowest available file descriptor, which is 0 (STDIN_FILENO).
    int newFd = open(pCmdLine->inputRedirect, O_RDONLY);
    if (newFd == -1) {
        perror("input redirection failed");
        _exit(1);
    }

}

//output redirect
if (pCmdLine->outputRedirect) {
    // Close STDOUT_FILENO (file descriptor 1).
    close(STDOUT_FILENO);

    // Open will now reuse the lowest available file descriptor, which is 1.
    int newFd = open(pCmdLine->outputRedirect, O_WRONLY);
    if (newFd == -1) {
        perror("output redirection failed");
        _exit(1);
    }

}

/*End of Task 3*/

/*Task 1a: print the process id of the child.*/
        fprintf(stderr, "PID: %d, Executing command %s\n",getpid(),pCmdLine->arguments[0]); // getpid from GPT
/*End of Task 1a*/

        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) { // Replace child process with new program
            perror("execvp failed"); // Print error message if execvp fails
            _exit(1); // Exit abnormally in the child process
        }
    } else { // Parent process

//Task 1c: wait for the child process only if blocking field is 1. else, run it on the background.
        if(pCmdLine->blocking){
            int status; // the child status that waitpid will update.
            waitpid(pid, &status, 0); // Wait for the child process to finish. 0 is default mode.
        }
        else{
            fprintf(stderr, "Child process %d running in the background\n", pid);
        }
/*End of Task 1c*/     
        
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

        cmdLine *parsedCmd = parseCmdLines(input); // Parse the input into a cmdLine structure
        if (parsedCmd == NULL) { // If parsing failed, continue to the next iteration
            continue;
        }

        if (strcmp(parsedCmd->arguments[0], "quit") == 0) { // Check if the user entered "quit"
            freeCmdLines(parsedCmd); // Free the allocated memory for parsedCmd
            break; // Exit the infinite loop
        }

// Every check : if (parsedCmd->argCount < 2) - from GPT.
/*Task 1b: add shell command "cd" and print appropriate error message if fails.*/
        // Handle the "cd" command
        if (strcmp(parsedCmd->arguments[0], "cd") == 0) {
            if (parsedCmd->argCount < 2) {
                fprintf(stderr, "cd: missing argument\n");
            } else {
                if (chdir(parsedCmd->arguments[1]) == -1) {//return -1 if couldn't change the directory
                    perror("cd failed"); // Print error if chdir fails
                }
            }
            //freeCmdLines(parsedCmd);  
            continue;
        }
/*End of Task 1b*/


//Task 2: Implement shell commands to help manage the processes using signals: 'stop', 'wake' and 'term'.
        
        // Handle "stop <pid>"
        if (strcmp(parsedCmd->arguments[0], "stop") == 0) {
            if (parsedCmd->argCount < 2) {
                fprintf(stderr, "stop: missing process ID\n");
            } else {
                pid_t pid = atoi(parsedCmd->arguments[1]); // Convert argument to PID
                if (kill(pid, SIGSTOP) == -1) {
                    perror("stop failed");
                }
            }
            //freeCmdLines(parsedCmd);
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
            }
            //freeCmdLines(parsedCmd);
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
            }
            //freeCmdLines(parsedCmd);
            continue;
        }
/*End of Task 2*/

        execute(parsedCmd); // Execute the parsed command
        freeCmdLines(parsedCmd); // Free the allocated memory for parsedCmd after execution
    }

    return 0; // Return 0 to indicate normal program terminationA
}
