#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

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

int main(int argc, char *argv[]) {
    struct sockaddr_in sockAddr;
    in_port_t destinationPort;

    if (argc < 2) {
        printf("Usage: %s destination_port", argv[0]);
    }
    destinationPort = (in_port_t) atoi(argv[1]);

    memset(&sockAddr, 0, sizeof(struct sockaddr_in));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(destinationPort);
    struct in_addr hostAddr = *(struct in_addr *) gethostbyname("localhost")->h_addr_list[0];
    memcpy(&sockAddr.sin_addr, &hostAddr, sizeof(struct in_addr));

    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error with socket");
        return -1;
    }

    int ret = connect(sock, (struct sockaddr *) &sockAddr, sizeof(sockAddr));
    if (ret == -1) {
        perror("Error occurred when trying to connect to other process");
        close(sock);
        return -1;
    }

    char buf[BUFSIZ];
    printf("Write to socket. 'EXIT' to end\n");
    if (writeSocket(sock, buf) == -1) {
        close(sock);
        return -1;
    }

    close(sock);
    return 0;
}