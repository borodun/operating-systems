#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

int fds[2];

void SIGINTHandler(int signum) {
    if (signum == SIGINT) {
        write(fds[1], "int", 3);
    } else {
        write(fds[1], "quit", 4);
    }
}

int main() {
    int flag = 1;
    int sigintCount = 0;
    if (pipe(fds) == -1) {
        perror("Error occurred while creating a pipe");
        return -1;
    }

    if (signal(SIGINT, SIGINTHandler) == SIG_ERR) {
        perror("Error occurred while mapping signal handler");
        return -1;
    }
    if (signal(SIGQUIT, SIGINTHandler) == SIG_ERR) {
        perror("Error occurred while mapping signal handler");
        return -1;
    }

    char signame[5];
    while (flag) {
        if (read(fds[0], signame, 5) > 0) {
            if (!memcmp("int", signame, 3)) {
                ++sigintCount;
            }
            if (!memcmp("quit", signame, 4)) {
                flag = 0;
            }
        }
    }

    printf("SIGINT count: %d\n", sigintCount);
    return 0;
}