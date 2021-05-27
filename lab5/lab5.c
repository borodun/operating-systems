#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int processFile(char *p, int *offsets, int *lenghts, int lineCount) {
    int line = 0;
    while (printf("Choose line to print(0 to exit): ") && scanf("%d", &line)) {
        if (line == 0) {
            return 0;
        }
        if (line < 0 || line > lineCount) {
            printf("Wrong line number: %d \n", line);
            continue;
        }

        int ret = write(1, p + offsets[line - 1], lenghts[line - 1]);
        if(ret == -1 || ret != lenghts[line - 1]){
            printf("Error with writing line %d, try again\n", line);
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

int readFile(int fd, char *p, int size) {
    int sum = 0;
    while (sum != size) {
        int n = read(fd, p + sum, size - sum);
        if (n == -1) {
            if (errno != EINTR || errno != EAGAIN || errno != EWOULDBLOCK) {
                return -1;
            }
        } else {
            sum += n;
        }
    }
    return 0;
}

int getSize(int fd) {
    lseek(fd, 0L, SEEK_END);
    int size = lseek(fd, 0L, SEEK_CUR);
    lseek(fd, 0L, SEEK_SET);
    return size;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s filename\n", argv[0]);
        return 0;
    }

    int file;
    if ((file = open(argv[1], O_RDONLY)) == -1) {
        perror("Error opening file");
        return 0;
    }

    int size = getSize(file);
    char *p = malloc(size * sizeof(char));
    if (p == NULL) {
        printf("Error with malloc\n");
        close(file);
        return 0;
    }

    int ret = readFile(file, p, size);
    if (ret == -1) {
        perror("error while reading file ");
        free(p);
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
    processFile(p, offsets, lengths, lineCount);

    free(offsets);
    free(lengths);
    free(p);
    close(file);
    return 0;
}