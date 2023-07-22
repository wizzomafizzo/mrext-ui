#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include "console.h"
#include "input.h"

enum actions read_key() {
    char c = '\0';

    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) {
        die("read");
    }

    if (c == 'q') {
        return ACTION_QUIT;
    }

    if (c == '\r' || c == '\n') {
        return ACTION_CONFIRM;
    }

    if (c == '\x1b') {
        char seq[2];

        if (read(STDIN_FILENO, &seq[0], 1) != 1) {
            return ACTION_NONE;
        }

        if (read(STDIN_FILENO, &seq[1], 1) != 1) {
            return ACTION_NONE;
        }

        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A':
                    return ACTION_UP;
                case 'B':
                    return ACTION_DOWN;
                case 'C':
                    return ACTION_RIGHT;
                case 'D':
                    return ACTION_LEFT;
            }
        }
    } else if (!iscntrl(c)) {
        return ACTION_NONE;
    }

    return ACTION_NONE;
}
