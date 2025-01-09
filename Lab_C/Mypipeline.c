#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pipe_fd[2]; // Array to hold pipe file descriptors
    pid_t child1, child2;

    // Debug message: parent process starts
    fprintf(stderr, "(parent_process > starting…)\n");

    // Step 1: Create a pipe
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(1);
    }
    fprintf(stderr, "(parent_process > pipe created)\n");

    // Step 2: Fork the first child
    fprintf(stderr, "(parent_process > forking…)\n");
    child1 = fork();
    if (child1 == -1) {
        perror("fork");
        exit(1);
    }

    if (child1 == 0) {
        // Step 3: First child process (child1)
        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe…)\n");

        close(STDOUT_FILENO);                 // Close standard output
        dup(pipe_fd[1]);                      // Redirect stdout to pipe write-end
        close(pipe_fd[1]);                    // Close the original write-end
        //close(pipe_fd[0]);                    // Close unused read-end

        // Execute "ls -l"
        fprintf(stderr, "(child1 > going to execute cmd: ls -l)\n");
        char *args[] = {"ls", "-l", NULL};
        execvp(args[0], args);

        // If execvp fails
        perror("execvp");
        exit(1);
    }

    // Parent process after first fork
    fprintf(stderr, "(parent_process > created process with id: %d)\n", child1);
    close(pipe_fd[1]); // Step 4: Close write-end in the parent
    fprintf(stderr, "(parent_process > closing the write end of the pipe…)\n");

    // Step 5: Fork the second child
    fprintf(stderr, "(parent_process > forking…)\n");
    child2 = fork();
    if (child2 == -1) {
        perror("fork");
        exit(1);
    }

    if (child2 == 0) {
        // Step 6: Second child process (child2)
        fprintf(stderr, "(child2 > redirecting stdin to the read end of the pipe…)\n");

        close(STDIN_FILENO);                  // Close standard input
        dup(pipe_fd[0]);                      // Redirect stdin to pipe read-end
        close(pipe_fd[0]);                    // Close the original read-end
        //close(pipe_fd[1]);                    // Close unused write-end

        // Execute "tail -n 2"
        fprintf(stderr, "(child2 > going to execute cmd: tail -n 2)\n");
        char *args[] = {"tail", "-n", "2", NULL};
        execvp(args[0], args);

        // If execvp fails
        perror("execvp");
        exit(1);
    }

    // Parent process after second fork
    fprintf(stderr, "(parent_process > created process with id: %d)\n", child2);
    close(pipe_fd[0]); // Step 7: Close read-end in the parent
    fprintf(stderr, "(parent_process > closing the read end of the pipe…)\n");

    // Step 8: Wait for both children to finish
    fprintf(stderr, "(parent_process > waiting for child processes to terminate…)\n");
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);
    fprintf(stderr, "(parent_process > exiting…)\n");

    return 0;
}
