#include <stdio.h>
#include <unistd.h>
#include <wait.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s filename \n", argv[0]);
        return -1;
    }

    pid_t deadChild, child;
    if ((child = fork()) == 0) {
        printf("Im child, printing some file\n");
        execl("/bin/cat", "cat", argv[1], NULL);
        perror("Error occurred with exec");
        return -1;
    } else if (child == -1) {
        perror("Error occurred when trying to fork a process ");
        return -1;
    }
    printf("Waiting for child with pid %d\n", child);

    int status;
    deadChild = wait(&status);
    if (deadChild == -1) {
        perror("Error occurred while waiting for child death ");
        return -1;
    }
    printf("Child died with pid %d\n", deadChild);

    return 0;
}