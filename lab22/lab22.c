#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <ctype.h>
#include <signal.h>

#define TIME_OUT 1

int alarmBeeped = 0;
FILE **fd;

int openFiles(int fileCount, char **files) {
    int openedFiles = 0;
    for (int i = 1; i < fileCount; i++) {
        printf("opening %s\n", files[openedFiles + 1]);

        if ((fd[openedFiles + 1] = fopen(files[openedFiles + 1], "rb")) == NULL) {
            printf("Cannot open %s\n", files[openedFiles + 1]);
            continue;
        } else {
            openedFiles++;
        }
    }
    return openedFiles;
}

void SIGALRMHandler(int sig) {
    alarmBeeped = 1;
}

void readFiles(int fileCount, char *argv[]) {
    signal(SIGALRM, SIGALRMHandler);
    siginterrupt(SIGALRM, 1);
    int openedFiles = fileCount;
    char buffer[BUFSIZ];
    char tmpBuffer[BUFSIZ];

    while (openedFiles) {
        for (int i = 0; i < fileCount; i++) {
            if (fd[i] == NULL) {
                continue;
            }

            alarm(TIME_OUT);

            if (fgets(buffer, BUFSIZ, fd[i]) == NULL && !alarmBeeped) {
                fclose(fd[i]);
                fd[i] = NULL;
                openedFiles--;
            } else if (alarmBeeped) {
                alarmBeeped = 0;
                continue;
            } else {
                sprintf(tmpBuffer, "%s: %s", argv[i], buffer);
                write(1, tmpBuffer, strlen(tmpBuffer));
                memset(buffer, 0, BUFSIZ);
                memset(tmpBuffer, 0, BUFSIZ);
            }
            alarm(0);
        }
    }
    signal(SIGALRM, SIG_DFL);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s files_to_read\n", argv[0]);
        return 0;
    }

    fd = (FILE **) malloc(argc * sizeof(FILE));
    if (fd == NULL) {
        perror("Error occurred with malloc\n");
        return -1;
    }
    int opened = openFiles(argc, argv);
    readFiles(opened, argv);
    
    return 0;
}