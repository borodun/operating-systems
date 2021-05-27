#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <math.h>
#include <netdb.h>
#include <errno.h>

#define MAX_FDS 1024

int getNewID(struct pollfd *fds) {
    for (int i = 1; i < MAX_FDS; i++) {
        if (fds[i].fd == -1) {
            return ceil(i / 2.0);
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
    if (argc < 4) {
        printf("Usage: %s proxy_port destination_port host_name\n", argv[0]);
        return -1;
    }

    printf("%d\n", getpid());
    in_port_t proxyPort = (in_port_t) atoi(argv[1]);
    in_port_t destPort = (in_port_t) atoi(argv[2]);

    struct hostent *hostInfo = gethostbyname(argv[3]);
    struct in_addr hostAddr = *(struct in_addr *) hostInfo->h_addr_list[0];

    printf("proxyPort: %d\n", proxyPort);
    printf("destPort: %d\n", destPort);
    printf("hostAddr: %d\n", hostAddr.s_addr);

    struct sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(proxyPort);
    sockAddr.sin_addr.s_addr = INADDR_ANY;

    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error with socket");
        return -1;
    }

    int flag = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) == -1) {
        perror("Error with setsockopt");
        return -1;
    }

    if (bind(sock, (struct sockaddr *) &sockAddr, sizeof(sockAddr)) < 0) {
        perror("Error with bind");
        close(sock);
        return -1;
    }
    
    int connectionNum = 510;
    if (listen(sock, connectionNum) != 0) {
        perror("Error with listen");
        close(sock);
        return -1;
    }

    struct pollfd fds[MAX_FDS];
    for (int i = 0; i < MAX_FDS; i++) {
        fds[i].fd = -1;
        fds[i].events = POLLIN;
        fds[i].revents = 0;
    }
    fds[1].fd = sock;

    int takenDescriptorsCount = 1;
    char buf[BUFSIZ];
    int maxID = 2;
    int timeout = 60 * 1000;
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
        
        for (int i = 1; i < maxID; ++i) {
            if (fds[1].revents & POLLIN) {
                int newConnection;
                if ((newConnection = accept(sock, NULL, NULL)) == -1) {
                    perror("accept");
                    continue;
                }

                int id;
                if ((id = getNewID(fds)) != -1) {
                    if (id > maxID) {
                        maxID = id;
                    }
                    
                    struct sockaddr_in destAddr;
                    memset(&destAddr, 0, sizeof(struct sockaddr_in));
                    destAddr.sin_family = AF_INET;
                    destAddr.sin_port = htons(destPort);
                    memcpy(&destAddr.sin_addr, &hostAddr, sizeof(struct in_addr));

                    int destSock;
                    if ((destSock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                        perror("Error with socket");
                        close(sock);
                        return -1;
                    }

                    if (connect(destSock, (struct sockaddr *) &destAddr, sizeof(struct sockaddr_in)) ==
                        -1) {
                        perror("Error with socket");
                        close(sock);
                        return -1;
                    }

                    fds[2 * id].fd = newConnection;
                    fds[2 * id + 1].fd = destSock;
                    takenDescriptorsCount += 2;
                } else {
                    close(newConnection);
                }
            } else {
                if (fds[i].revents & POLLIN) {
                    int ret = readSocket(fds[i].fd, buf);
                    if (ret == -1) {
                        close(fds[i].fd);
                        close(fds[i + 1].fd);
                        fds[i].fd = -1;
                        fds[i + 1].fd = -1;
                        break;
                    }
                    if (i % 2 == 1) {
                        write(fds[i - 1].fd, buf, ret);
                    } else {
                        write(fds[i + 1].fd, buf, ret);
                    }
                }
            }
        }
    }

    for (int i = 0; i < MAX_FDS; ++i) {
        if (fds[i].fd != -1) {
            close(fds[i].fd);
        }
    }
    close(sock);
    return 0;
}