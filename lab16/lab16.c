#include <termios.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    int tty = STDIN_FILENO;
    if (!isatty(tty)) {
        printf("not a tty\n");
        return -1;
    }

    struct termios orig;
    struct termios raw;
    tcgetattr(tty, &orig);

    raw = orig;
    raw.c_lflag &= ~ICANON;
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(tty, TCSANOW, &raw);

    printf("Yandex or Google [y\\g]?\n");
    char letter;
    read(tty, &letter, 1);
    if (letter == 'y') {
        printf("\nYandex\n");
    } else if (letter == 'g') {
        printf("\nGoogle\n");
    }

    tcsetattr(tty, TCSANOW, &orig);
    return 0;
}