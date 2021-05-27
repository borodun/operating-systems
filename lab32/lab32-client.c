#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int writeSocket(int sock, char *buf) {
    int readCount, writeCount;
    char *exit = "EXIT";

    while (1) {
        readCount = read(0, buf, sizeof(buf));
        if (readCount > 0) {
            writeCount = write(sock, buf, readCount);
            if (writeCount == -1) {
                if (errno != EWOULDBLOCK) {
                    perror("Error with write");
                    return -1;
                }
            }
            if (strstr(buf, exit) != NULL) {
                return 0;
            }
        }
        if (readCount == -1) {
            if (errno != EWOULDBLOCK) {
                perror("Error with read");
                return -1;
            }
        }
    }
}

int main() {
    struct sockaddr_un sockAddr;
    char *sockPath = "./socket";
    char buf[BUFSIZ];

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Error occurred while creating socket");
        return -1;
    }

    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sun_family = AF_UNIX;
    strncpy(sockAddr.sun_path, sockPath, sizeof(sockAddr.sun_path) - 1);

    int ret = connect(sock, (struct sockaddr *) &sockAddr, sizeof(sockAddr));
    if (ret == -1) {
        perror("Error occurred when trying to connect to other process");
        close(sock);
        return -1;
    }

    printf("Write to socket. 'EXIT' to end\n");
    if (writeSocket(sock, buf) == -1) {
        close(sock);
        return -1;
    }

    close(sock);
    return 0;
}
