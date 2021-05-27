#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    struct flock lock;
    char command[100];

    if (argc < 2) {
        printf("Usage: %s filename \n", argv[0]);
        return -1;
    }

    int file;
    if ((file = open(argv[1], O_RDWR)) == -1) {
        perror("Can't open file");
        return -1;
    }

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(file, F_SETLKW, &lock) == -1) {
        perror("Error occurred when trying to lock a file ");
        close(file);
        return -1;
    }
    snprintf(command, 100, "mcedit %s\n", argv[1]);

    int status = system(command);
    if (status == -1) {
        printf("Error with system \n");
        return -1;
    }
    if (WIFEXITED(status)) {
        printf("Exit status %d\n", WEXITSTATUS(status));
    }

    lock.l_type = F_UNLCK;
    fcntl(file, F_SETLK, &lock);
    close(file);
    return 0;
}