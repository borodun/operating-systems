#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s text_for_program \n", argv[0]);
        return -1;
    }
    unsigned long fullLength = 0;
    for (int i = 1; i < argc; ++i) {
        fullLength += strlen(argv[i]) + 1;
    }
    fullLength++;
    char command[32];
    snprintf(command, 32, "./lab26-2 %lu", fullLength);

    FILE *pipe = popen(command, "w");
    if (pipe == NULL) {
        perror("Error occurred with popen");
        return -1;
    }

    char buf[fullLength];
    unsigned long offset = 0;
    for (int i = 1; i < argc; ++i) {
        unsigned long len = strlen(argv[i]) + 2;
        snprintf(buf + offset, len, "%s ", argv[i]);
        offset += len - 1;
    }
    fwrite(buf, sizeof(char), fullLength, pipe);

    if (pclose(pipe) == -1) {
        printf("Error occurred with pclose\n");
        return -1;
    }
    return 0;
}