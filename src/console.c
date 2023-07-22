#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "console.h"

struct termios orig_term;

void die(const char *s) {
    perror(s);
    exit(1);
}

void disable_raw_mode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_term) == -1) {
        die("tcsetattr");
    }

    printf("\033[?25h");
    printf("\033[2J");
    printf("\033[H");
}

void enable_raw_mode() {
    if (tcgetattr(STDIN_FILENO, &orig_term) == -1) {
        die("tcgetattr");
    }

    atexit(disable_raw_mode);

    struct termios raw = orig_term;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    setbuf(stdin, NULL);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        die("tcsetattr");
    }

    printf("\033[?25l");
}
