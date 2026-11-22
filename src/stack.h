#ifndef STACK_H
#define STACK_H

#include <stddef.h>
#include <stdio.h>

typedef struct stack {
    long *v; /* v[n - 1] is the top */
    size_t n;
    size_t cap;
} stack;

void stack_push(stack *s, long x);

/* Caller checks that the stack isn't empty. */
long stack_pop(stack *s);

/* Buries the top value `depth` deep and brings the values above it up,
   `rolls` times (negative: the other way). 0 <= depth <= size. */
void stack_roll(stack *s, long depth, long rolls);

void stack_print(const stack *s, FILE *f);

void stack_free(stack *s);

#endif
