#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

int fds[2];
struct termios orig;
struct termios raw;

void SIGINTHandler(int signum) {
    if (signum == SIGINT) {
        write(fds[1], "int", 3);
    }
}

int main(int argc, char *argv[]) {
    if (pipe(fds) == -1) {
        perror("Error occurred while creating a pipe");
        return -1;
    }
    fcntl(fds[0], F_SETFL, O_NONBLOCK);

    if (signal(SIGINT, SIGINTHandler) == SIG_ERR) {
        perror("Error occurred while mapping signal handler");
        return -1;
    }

    int tty = STDOUT_FILENO;
    if (!isatty(tty)) {
        printf("not a tty\n");
        return -1;
    }

    tcgetattr(tty, &orig);

    raw = orig;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    tcsetattr(tty, TCSANOW, &raw);

    printf("\tType text(40 chars per line):"
           "\n\tBACKSPACE to erase char,"
           "\n\tCTRL-U to erase line, "
           "\n\tCTRL-W to erase word"
           "\n\tCTRL-D to finish typing\n");
    char signame[5];
    int line = 0;
    int position = 0;
    while (1) {
        if (read(fds[0], signame, 5) > 0) {
            if (!memcmp("int", signame, 3)) {
                printf("\nINTERRUPTED\n");
                break;
            }
        }
        char c;
        int ret = read(tty, &c, 1);
        if (ret > 0) {
            if (c == 4) { // CTRL-D (EXIT)
                if (position == 0) {
                    break;
                }
                continue;
            }
            if (c == 21) { // CTRL-U (ERASE LINE)
                if (position != 0) {
                    char killBuf[position * 3 + 2];
                    char delBuf[3] = "\b \b";

                    unsigned long offset = 0;
                    for (int i = 0; i < position; ++i) {
                        strncpy(killBuf + offset, delBuf, 4);
                        offset += 3;
                    }
                    strncpy(killBuf + offset, "\r", 2);

                    write(STDOUT_FILENO, killBuf, sizeof(killBuf));
                    position = 0;
                }
                continue;
            }
            if (c == 23) { // CTRL-W (ERASE WORD)
                if (position != 0) {
                    char buf[40];
                    lseek(tty, -position, SEEK_CUR);
                    read(tty, buf, sizeof(buf));
                    lseek(tty, position, SEEK_CUR);

                    int workLength = 0;
                    for (int i = strlen(buf) - 1; i > 0 && (isalpha(buf[i]) || isdigit(buf[i])); --i) {
                        ++workLength;
                    }

                    char eraseBuf[workLength * 3 + 1];
                    char delBuf[3] = "\b \b";

                    unsigned long offset = 0;
                    for (int i = 0; i < workLength; ++i) {
                        strncpy(eraseBuf + offset, delBuf, 4);
                        offset += 3;
                    }

                    write(STDOUT_FILENO, eraseBuf, sizeof(eraseBuf));
                }
                continue;
            }
            if (c == 127) { // BACKSPACE (ERASE SYMBOL)
                if (position != 0) {
                    char delBuf[3] = "\b \b";
                    write(STDOUT_FILENO, delBuf, sizeof(delBuf));
                    --position;
                }
                continue;
            }

            write(STDOUT_FILENO, &c, 1);
            position += ret;
            if (position >= 40) {
                write(STDOUT_FILENO, "\n", 1);
                position = 0;
                ++line;
            }
            if (c == '\n') {
                position = 0;
                ++line;
            }
        }
    }

    tcsetattr(tty, TCSANOW, &orig);
    return 0;
}