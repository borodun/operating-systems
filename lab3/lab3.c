#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    FILE *file;

    if (argc < 2){
        printf("Usage: %s filename\n", argv[0]);
        return 0;
    }

    printf("UID: %d \tEUID: %d\n", getuid(), geteuid());
    file = fopen(argv[1], "r");
    if (!file) {
        perror("Error opening file");
        return 0;
    } else {
        printf("File opened \n");
        fclose(file);
    }

    int ret = seteuid(getuid());
    if (ret == -1){
        perror("Error with seteuid");
    }
    printf("\nUID: %d \tEUID: %d\n", getuid(), geteuid());

    file = fopen(argv[1], "r");
    if (!file) {
        perror("Error opening file");
        return 0;
    } else {
        printf("File opened \n");
        fclose(file);
    }
    return 0;
}