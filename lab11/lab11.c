#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

extern char **environ;

int execvpe(char *prog, char *argv[], char *envp[]) {
    if (prog == NULL) {
        printf("No program to execute\n");
        return -2;
    }

    char **savedEnviron = environ;
    environ = envp;

    execvp(prog, argv);
    perror("Error with exec");
    environ = savedEnviron;
    return -1;
}

int main(int argc, char *argv[], char *envp[]) {
    if (argc < 2) {
        printf("Usage: %s program_name program_args \n", argv[0]);
        return -1;
    }

    if (putenv("PATH=.") == -1) {
        perror("Error with putenv");
        return -1;
    }

    if (execvpe(argv[1], &argv[1], envp) == -1) {
        return -1;
    }
}