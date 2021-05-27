#include <stdio.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/un.h>
#include <string.h>

#define MAX_FDS 10

int getNewID(struct pollfd *fds) {
    for (int i = 1; i < MAX_FDS; i++) {
        if (fds[i].fd == -1) {
            return i;
        }
    }
    return -1;
}

int readSocket(int sd, char *buf) {
    int readCount;
    char *exit = "EXIT";

    readCount = read(sd, buf, sizeof(buf));
    if (readCount <= 0) {
        if (errno == EWOULDBLOCK) {
            return 0;
        }
        return -1;
    }

    if (strstr(buf, exit) != NULL) {
        return -1;
    }
    for (int i = 0; i < readCount; i++) {
        buf[i] = toupper(buf[i]);
    }
    printf("%.*s", readCount, buf);
    return 0;
}

int main() {
    struct sockaddr_un sockAddr;
    int listenSock;
    char buf[BUFSIZ];
    char *socket_path = "./socket";
    unlink(socket_path);

    if ((listenSock = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1) {
        perror("Error occurred while creating a socket");
        return -1;
    }

    memset(&sockAddr, 0, sizeof(sockAddr));
    strncpy(sockAddr.sun_path, socket_path, sizeof(sockAddr.sun_path) - 1);
    sockAddr.sun_family = AF_UNIX;

    if (bind(listenSock, (struct sockaddr *) &sockAddr, sizeof(sockAddr)) == -1) {
        perror("Error occurred with bind");
        close(listenSock);
        return -1;
    }

    if (listen(listenSock, MAX_FDS) == -1) {
        perror("Error occurred with listen");
        close(listenSock);
        return -1;
    }

    struct pollfd fds[MAX_FDS];
    for (int i = 0; i < MAX_FDS; i++) {
        fds[i].fd = -1;
        fds[i].events = POLLIN;
        fds[i].revents = 0;
    }
    fds[0].fd = listenSock;

    int maxID = 1;
    int timeout = 5 * 1000;
    int end = 0;
    while (!end) {
        int pollRet = poll(fds, maxID, timeout);
        if (pollRet == -1) {
            perror("Error occurred with poll");
            break;
        }
        if (pollRet == 0) {
            printf("TIMEOUT\n");
            break;
        }

        for (int i = 0; i < maxID; i++) {
            if (!(fds[i].revents & POLLIN) || fds[i].fd == -1) {
                continue;
            }

            if (fds[i].fd == listenSock && maxID < MAX_FDS) {
                int newConnection = accept(listenSock, NULL, NULL);
                if (newConnection == -1) {
                    if (errno != EWOULDBLOCK) {
                        perror("Error occurred with accept");
                        end = 1;
                        break;
                    }
                }
                int id;
                if ((id = getNewID(fds)) != -1) {
                    if (id > maxID) {
                        maxID = id;
                    }
                    fds[id].fd = newConnection;
                } else {
                    close(newConnection);
                }
            } else {
                int ret = readSocket(fds[i].fd, buf);
                if (ret == -1) {
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    fds[i].events = 0;
                    break;
                }
            }
        }
    }
    close(listenSock);
    return 0;
}