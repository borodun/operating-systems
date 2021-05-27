#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <aio.h>
#include <signal.h>
#include <errno.h>
#include <sys/un.h>

#define MAX_CLIENTS 66

struct Client {
    int id;
    struct aiocb block;
    char buf[BUFSIZ];
};

struct Client clients[MAX_CLIENTS];

void process(int index) {
    char *exit = "EXIT";
    long size = aio_return(&clients[index].block);
    if (size == 0 || strstr(clients[index].buf, exit) != NULL) {
        close(clients[index].block.aio_fildes);
        clients[index].id = 0;
        return;
    } else {
        for (long i = 0; i < size; i++) {
            clients[index].buf[i] = toupper(clients[index].buf[i]);
        }
        write(STDOUT_FILENO, clients[index].buf, size);
        aio_read(&clients[index].block);
    }
}

void asyncRead(int i, int fd) {
    clients[i].id = i;
    clients[i].block.aio_fildes = fd;
    clients[i].block.aio_offset = 0;
    clients[i].block.aio_buf = clients[i].buf;
    clients[i].block.aio_nbytes = BUFSIZ;
    clients[i].block.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    clients[i].block.aio_sigevent.sigev_signo = SIGIO;
    clients[i].block.aio_sigevent.sigev_value.sival_int = i;
    aio_read(&clients[i].block);
}

static void SIGIOHandler(int signum, siginfo_t *siginfo, void *context) {
    int index = siginfo->si_value.sival_int;
    if (aio_error(&clients[index].block) == 0) {
        process(index);
    } else {
        printf("Error occurred while reading from client %d", clients[index].id = index);
    }
}

int getNewID() {
    for (int i = 1; i < MAX_CLIENTS; i++) {
        if (clients[i].id == 0) {
            return i;
        }
    }
    return -1;
}

int main() {
    struct sockaddr_un sockAddr;
    char *sockPath = "./socket";
    unlink(sockPath);

    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sun_family = AF_UNIX;
    strcpy(sockAddr.sun_path, sockPath);

    int sock;
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    if (bind(sock, (struct sockaddr *) &sockAddr, sizeof(sockAddr)) < 0) {
        perror("bind");
        close(sock);
        return -1;
    }

    if (listen(sock, MAX_CLIENTS) != 0) {
        perror("listen");
        unlink(sockAddr.sun_path);
        close(sock);
        return -1;
    }

    sigset_t sigIO;
    sigaddset(&sigIO, SIGIO);

    struct sigaction sigIOAction;
    sigemptyset(&sigIOAction.sa_mask);
    sigIOAction.sa_sigaction = SIGIOHandler;
    sigIOAction.sa_mask = sigIO;
    sigIOAction.sa_flags = SA_SIGINFO;
    sigaction(SIGIO, &sigIOAction, NULL);

    memset(clients, 0, MAX_CLIENTS * sizeof(struct Client));
    while (1) {
        int newConnection;
        if ((newConnection = accept(sock, NULL, NULL)) == -1) {
            if (errno != EWOULDBLOCK) {
                break;
            }
        }

        int newID;
        if ((newID = getNewID()) != -1) {
            asyncRead(newID, newConnection);
        } else {
            close(newConnection);
        }
    }

    close(sock);
    return 0;
}