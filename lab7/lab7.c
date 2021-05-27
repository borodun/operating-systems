#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <poll.h>
#include <sys/mman.h>
#include <memory.h>

int processFile(char *p, int *offsets, int size, int *lenghts, int lineCount) {
    struct pollfd pfd;
    pfd.fd = 0;
    pfd.events = POLLIN;

    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    int previousTime = currentTime.tv_sec;

    int line = 0;
    printf("Choose line to print in 5 seconds(0 to exit): \n");
    while (1) {
        if (poll(&pfd, 1, 100) > 0) {
            scanf("%d", &line);
            if (line == 0) {
                return 0;
            }

            gettimeofday(&currentTime, NULL);
            if (line < 0 || line > lineCount) {
                previousTime = currentTime.tv_sec;
                printf("Wrong line number: %d \n", line);
                printf("Choose line to print in 5 seconds(0 to exit): \n");
                continue;
            }
            previousTime = currentTime.tv_sec;

            int ret = write(1, p + offsets[line - 1], lenghts[line - 1]);
            if(ret == -1 || ret != lenghts[line - 1]){
                printf("Error with writing line %d, try again\n", line);
            }

            printf("Choose line to print in 5 seconds(0 to exit): \n");
        }

        gettimeofday(&currentTime, NULL);
        if ((currentTime.tv_sec - previousTime) >= 5) {
            write(1, p, size);
            printf("Timeout exceeded\n");
            break;
        }
    }
    return 0;
}

void parseFile(const char *p, int size, int *offsets, int *lengths) {
    offsets[0] = 0;
    int tmp = 1;
    for (int i = 0; i < size; ++i) {
        if (p[i] == '\n') {
            lengths[tmp - 1] = i - offsets[tmp - 1] + 1;
            offsets[tmp] = i + 1;
            ++tmp;
        }
    }
}

int countLines(const char *p, int size) {
    int lineCount = 0;
    for (int i = 0; i < size; ++i) {
        if (p[i] == '\n') {
            ++lineCount;
        }
    }
    return lineCount;
}

int getSize(int fd) {
    lseek(fd, 0L, SEEK_END);
    int size = lseek(fd, 0L, SEEK_CUR);
    lseek(fd, 0L, SEEK_SET);
    return size;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s filename \n", argv[0]);
        return 0;
    }

    int file;
    if ((file = open(argv[1], O_RDONLY)) == -1) {
        perror("Error opening file");
        return 0;
    }

    int size = getSize(file);
    char *p = mmap(0, size, PROT_READ, MAP_SHARED, file, 0);
    if (p == MAP_FAILED) {
        perror("Error with mmap ");
        close(file);
        return 0;
    }

    int lineCount = countLines(p, size);
    int *offsets = malloc((lineCount + 1) * sizeof(int));
    if (offsets == NULL) {
        printf("Error with malloc\n");
        free(p);
        close(file);
        return 0;
    }
    int *lengths = malloc((lineCount + 1) * sizeof(int));
    if (lengths == NULL) {
        printf("Error with malloc\n");
        free(offsets);
        free(p);
        close(file);
        return 0;
    }

    parseFile(p, size, offsets, lengths);
    printf("Line count: %d\n", lineCount);
    processFile(p, offsets, size, lengths, lineCount);

    munmap(p, size);
    free(offsets);
    free(lengths);
    close(file);
    return 0;
}