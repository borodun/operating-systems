#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

int alarmed = 0;
int fds[2];

void SIGHandler(int signum) {
    if (signum == SIGINT) {
        write(fds[1], "int", 3);
    } else {
        alarmed = 1;
    }
}

int openFiles(int fileCount, char *argv[], FILE *fd[]) {
    int openedFiles = 0;
    for (int i = 1; i < fileCount; i++) {
        printf("Opening %s\n", argv[i]);

        if ((fd[openedFiles] = fopen(argv[i], "r")) == NULL) {
            printf("Cannot open %s\n", argv[i]);
            continue;
        } else {
            openedFiles++;
        }
    }
    return openedFiles;
}

void readFiles(int fileCount, FILE *fd[]) {
    signal(SIGALRM, SIGHandler);
    signal(SIGINT, SIGHandler);

    int openedFiles = fileCount;
    char buf[BUFSIZ];

    char signame[5];
    int timeout = 1;
    while (openedFiles) {
        for (int i = 0; i < fileCount; i++) {
            if (fd[i] == NULL) {
                continue;
            }
            if (read(fds[0], signame, 5) > 0) {
                if (!memcmp("int", signame, 3)) {
                    for (int j = 0; j < fileCount; ++j) {
                        if (fd[j]) {
                            fclose(fd[j]);
                            openedFiles--;
                        }
                    }
                    break;
                }
            }

            alarm(timeout);

            char *ret = fgets(buf, BUFSIZ, fd[i]);

            if (ret != NULL && !alarmed) {
                write(1, buf, strlen(buf));
            } else if (alarmed) {
                alarmed = 0;
                continue;
            } else {
                fclose(fd[i]);
                fd[i] = NULL;
                openedFiles--;
            }

            alarm(0);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s files_to_read\n", argv[0]);
        return 0;
    }

    if (pipe(fds) == -1) {
        perror("Error occurred while creating a pipe");
        return -1;
    }
    fcntl(fds[0], F_SETFL, O_NONBLOCK);

    FILE **fd = malloc(argc * sizeof(FILE));
    if (fd == NULL) {
        perror("Error occurred with malloc\n");
        return -1;
    }

    int opened = openFiles(argc, argv, fd);
    if (opened) {
        readFiles(opened, fd);
    }

    free(fd);
    return 0;
}