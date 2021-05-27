#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s file \n", argv[0]);
        return -1;
    }
    unsigned long len = strlen(argv[1]) + 16;
    char command[len];
    snprintf(command, len, "grep '^$' %s| wc -l", argv[1]);

    FILE *pipe = popen(command, "r");
    if (pipe == NULL) {
        perror("Error occurred with popen");
        return -1;
    }

    int lineCount;
    if (fscanf(pipe, "%d", &lineCount) != 1) {
        printf("Error with fscanf\n");
        return -1;
    }
    printf("Amount of empty lines is %d\n", lineCount);

    if (pclose(pipe) == -1) {
        printf("Error occurred with pclose\n");
        return -1;
    }
    return 0;
}