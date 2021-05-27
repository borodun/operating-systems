#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

int readStream(char *buf, unsigned long len) {
    long sum = 0;
    while (sum != len) {
        long n = read(STDIN_FILENO, buf + sum, len - sum);
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
    unsigned long len = atoi(argv[1]);
    char buf[len];

    readStream(buf, len);

    for (int i = 0; i < len; ++i) {
        buf[i] = (char) toupper(buf[i]);
    }
    printf("%s\n", buf);

    return 0;
}