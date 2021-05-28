#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <errno.h>

#define MAX_FDS 1024

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
    printf("%.*s", readCount, buf);
    return readCount;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s destination_port\n", argv[0]);
    }

    struct sockaddr_in sockAddr;
    in_port_t destinationPort;
    destinationPort = (in_port_t) atoi(argv[1]);

    memset(&sockAddr, 0, sizeof(struct sockaddr_in));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(destinationPort);

    int listenSock;
    if ((listenSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error occurred while creating socket");
        return -1;
    }

    int flag = 1;
    if (setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) == -1) {
        perror("Error occurred with setsockopt");
        close(listenSock);
        return -1;
    }

    if (bind(listenSock, (struct sockaddr *) &sockAddr, sizeof(struct sockaddr_in)) < 0) {
        perror("Error with bind");
        close(listenSock);
        return -1;
    }

    int connectionNum = 510;
    if (listen(listenSock, connectionNum) != 0) {
        perror("Error with listen");
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

    int timeout = 60 * 1000;
    int maxID = 1;
    char buf[BUFSIZ];
    memset(&buf, 0, sizeof(buf));

    while (true) {
        int pollRet = poll(fds, MAX_FDS, timeout);
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
                    break;
                }
                write(fds[i].fd, buf, ret);
            }
        }
    }

    for (int i = 0; i < MAX_FDS; ++i) {
        if (fds[i].fd != -1) {
            close(fds[i].fd);
        }
    }
    close(listenSock);
    return 0;
}