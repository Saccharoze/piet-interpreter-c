#ifndef COMMANDS_H
#define COMMANDS_H

#include "stack.h"

/* Index = hue change * 3 + lightness change, from one block to the next. */
enum command {
    CMD_NONE, CMD_PUSH, CMD_POP,
    CMD_ADD, CMD_SUBTRACT, CMD_MULTIPLY,
    CMD_DIVIDE, CMD_MOD, CMD_NOT,
    CMD_GREATER, CMD_POINTER, CMD_SWITCH,
    CMD_DUPLICATE, CMD_ROLL, CMD_IN_NUMBER,
    CMD_IN_CHAR, CMD_OUT_NUMBER, CMD_OUT_CHAR,
};

/* The command run when leaving a block of colour `from` for one of colour `to`. */
int command_between(int from, int to);

const char *command_name(int cmd);

/* Runs a command. `size` is the size of the block being left (for push).
   A command that can't run (stack too short, division by zero...) does
   nothing. */
void execute(int cmd, stack *s, long size, int *dp, int *cc);

#endif
