#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

int writePipe(int fd, char *buf, unsigned long len) {
    long sum = 0;
    while (sum != len) {
        long n = write(fd, buf + sum, len - sum);
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

int readPipe(int fd, char *buf, unsigned long len) {
    long sum = 0;
    while (sum != len) {
        long n = read(fd, buf + sum, len - sum);
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

    int fds[2];
    if (pipe(fds) == -1) {
        perror("Error occurred while creating a pipe");
        return -1;
    }

    char buf[fullLength];
    unsigned long offset = 0;
    for (int i = 1; i < argc; ++i) {
        unsigned long len = strlen(argv[i]) + 2;
        snprintf(buf + offset, len, "%s ", argv[i]);
        offset += len - 1;
    }

    pid_t proc1;
    if ((proc1 = fork()) == 0) {
        close(fds[0]);
        if (writePipe(fds[1], buf, fullLength) == -1) {
            perror("Error occurred with write");
            close(fds[1]);
            return -1;
        }
        close(fds[1]);
        return 0;
    } else if (proc1 == -1) {
        perror("Error occurred with fork1");
        close(fds[0]);
        close(fds[1]);
        return -1;
    }

    pid_t proc2;
    if ((proc2 = fork()) == 0) {
        close(fds[1]);
        char str[fullLength];
        if (readPipe(fds[0], str, fullLength) == -1) {
            perror("Error occurred with read");
            return -1;
        }
        for (int i = 0; i < fullLength; ++i) {
            str[i] = (char) toupper(buf[i]);
        }
        printf("%s\n", str);
        return 0;
    } else if (proc2 == -1) {
        perror("Error occurred with fork2");
        close(fds[0]);
        close(fds[1]);
        return -1;
    }

    sleep(1);
    int status1, ret1 = waitpid(proc1, &status1, WNOHANG);
    if (ret1 == -1) {
        perror("Error occurred while waiting for child process");
        return -1;
    }
    if (WIFEXITED(status1)) {
        printf("1st process finisihed with exit code %d\n", WEXITSTATUS(status1));
    } else {
        printf("1st process haven't finished properly \n");
    }

    int status2, ret2 = waitpid(proc2, &status2, WNOHANG);
    if (ret2 == -1) {
        perror("Error occurred while waiting for child process");
        return -1;
    }
    if (WIFEXITED(status2)) {
        printf("2nd process finisihed with exit code %d\n", WEXITSTATUS(status2));
    } else {
        printf("2nd process haven't finished properly \n");
    }

    return 0;
}