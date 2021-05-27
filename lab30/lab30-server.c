#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

int readSocket(int sd, char *buf) {
    int readCount;
    char *exit = "EXIT";

    while (1) {
        while ((readCount = read(sd, buf, sizeof(buf))) > 0) {
            if (strstr(buf, exit) != NULL) {
                return -1;
            }
            for (int i = 0; i < readCount; i++) {
                buf[i] = toupper(buf[i]);
            }
            printf("%.*s", readCount, buf);
        }
        if (readCount == -1) {
            if (errno != EINTR || errno != EAGAIN || errno != EWOULDBLOCK) {
                return -1;
            }
        } else if (readCount == 0) {
            return 0;
        }
    }
}

int main() {
    struct sockaddr_un sockAddr;
    char buf[BUFSIZ];
    char *sockPath = "./socket";
    unlink(sockPath);

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Error occurred while creating socket");
        return -1;
    }

    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sun_family = AF_UNIX;
    strncpy(sockAddr.sun_path, sockPath, sizeof(sockAddr.sun_path) - 1);

    int res = bind(sock, (struct sockaddr *) &sockAddr, sizeof(sockAddr));
    if (res == -1) {
        perror("Error occurred with bind");
        close(sock);
        return -1;
    }

    if (listen(sock, 1) == -1) {
        perror("Error occurred with listen");
        close(sock);
        return -1;
    }

    int sd = accept(sock, NULL, NULL);
    if (sd == -1) {
        if (errno != EWOULDBLOCK) {
            perror("Error occurred with accept");
            close(sock);
            return -1;
        }
    }

    int ret = readSocket(sd, buf);
    if (ret == -1) {
        close(sd);
        close(sock);
        return -1;
    }

    close(sd);
    close(sock);
    return 0;
}