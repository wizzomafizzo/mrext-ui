#ifndef RADICAL_INPUT_H
#define RADICAL_INPUT_H

enum actions {
    ACTION_NONE,
    ACTION_LEFT,
    ACTION_RIGHT,
    ACTION_UP,
    ACTION_DOWN,
    ACTION_QUIT,
    ACTION_CONFIRM,
};

enum actions read_key();

#endif //RADICAL_INPUT_H
