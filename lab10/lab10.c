#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s program_name args_for_program \n", argv[0]);
        return -1;
    }

    pid_t deadChild, child;
    if ((child = fork()) == 0) {
        execvp(argv[1], &argv[1]);
        perror("Error occurred when trying to execute a program");
        return -1;
    } else if (child == -1) {
        perror("Error occurred when trying to fork a process");
        return -1;
    }
    printf("Waiting for child with pid %d\n", child);

    int status;
    deadChild = wait(&status);
    if (deadChild == -1) {
        perror("Error occurred while waiting for child death ");
        return -1;
    }
    if (WIFEXITED(status)) {
        printf("Exit status of child with pid %d: %d\n", deadChild, WEXITSTATUS(status));
    }
    return 0;
}