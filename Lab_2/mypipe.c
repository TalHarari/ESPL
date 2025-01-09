#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {
    int pipefd[2]; // Array to hold the file descriptors for the pipe
    pid_t pid;
    char message[] = "Hello! It's me, Asaf Belilus!"; // Message to be sent from child to parent
    char buffer[128]; // Buffer to hold the message read by the parent

    // Create the pipe
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        exit(1);
    }

    // Create the child process
    pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) { // Child process

        close(pipefd[0]); // Close the read end of the pipe
        write(pipefd[1], message, strlen(message) + 1); // Write the message to the pipe
        close(pipefd[1]); // Close the write end of the pipe
        printf("Child proccess sent the message! \n");
        exit(0);
    } 

    else { // Parent process

        close(pipefd[1]); // Close the write end of the pipe
        read(pipefd[0], buffer, sizeof(buffer)); // Read the message from the pipe
        printf("Parent received: %s\n", buffer); // Print the message
        close(pipefd[0]); // Close the read end of the pipe
    }

    return 0;
}
