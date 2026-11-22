#include <ctype.h>
#include <stdio.h>
#include "colour.h"
#include "commands.h"

int command_between(int from, int to)
{
    int hue = (HUE(to) - HUE(from) + 6) % 6;
    int light = (LIGHTNESS(to) - LIGHTNESS(from) + 3) % 3;
    return hue * 3 + light;
}

const char *command_name(int cmd)
{
    static const char *names[18] = {
        "none", "push", "pop",
        "add", "subtract", "multiply",
        "divide", "mod", "not",
        "greater", "pointer", "switch",
        "duplicate", "roll", "in(number)",
        "in(char)", "out(number)", "out(char)",
    };
    return cmd >= 0 && cmd < 18 ? names[cmd] : "?";
}

/* Arithmetic wraps around instead of overflowing. */
static long wrap(unsigned long x)
{
    return (long)x;
}

/* Division rounds down, so that b == a * (b / a) + b % a with mod. */
static long divide(long b, long a)
{
    if (a == -1)
        return wrap(0UL - (unsigned long)b);
    long q = b / a;
    if (b % a != 0 && (b < 0) != (a < 0))
        q--;
    return q;
}

/* The result has the sign of the divisor `a`. */
static long mod(long b, long a)
{
    if (a == -1)
        return 0;
    long r = b % a;
    if (r != 0 && (r < 0) != (a < 0))
        r += a;
    return r;
}

static int read_number(long *out)
{
    int c;
    do
        c = getchar();
    while (c != EOF && isspace(c));
    int negative = 0;
    if (c == '-' || c == '+') {
        negative = c == '-';
        c = getchar();
    }
    if (c == EOF || !isdigit(c)) {
        if (c != EOF)
            ungetc(c, stdin);
        return 0;
    }
    unsigned long n = 0;
    while (c != EOF && isdigit(c)) {
        n = n * 10 + (unsigned long)(c - '0');
        c = getchar();
    }
    if (c != EOF)
        ungetc(c, stdin);
    *out = negative ? wrap(0UL - n) : wrap(n);
    return 1;
}

/* One character, UTF-8 decoded. */
static int read_char(long *out)
{
    int c = getchar();
    if (c == EOF)
        return 0;
    int extra = c >= 0xF0 ? 3 : c >= 0xE0 ? 2 : c >= 0xC0 ? 1 : 0;
    long code = extra == 3 ? c & 0x07 : extra == 2 ? c & 0x0F : extra == 1 ? c & 0x1F : c;
    for (; extra > 0; extra--) {
        int d = getchar();
        if (d == EOF || (d & 0xC0) != 0x80) {
            if (d != EOF)
                ungetc(d, stdin);
            break;
        }
        code = (code << 6) | (d & 0x3F);
    }
    *out = code;
    return 1;
}

static void write_char(long code)
{
    if (code < 0x80) {
        putchar((int)code);
    } else if (code < 0x800) {
        putchar((int)(0xC0 | (code >> 6)));
        putchar((int)(0x80 | (code & 0x3F)));
    } else if (code < 0x10000) {
        putchar((int)(0xE0 | (code >> 12)));
        putchar((int)(0x80 | ((code >> 6) & 0x3F)));
        putchar((int)(0x80 | (code & 0x3F)));
    } else {
        putchar((int)(0xF0 | (code >> 18)));
        putchar((int)(0x80 | ((code >> 12) & 0x3F)));
        putchar((int)(0x80 | ((code >> 6) & 0x3F)));
        putchar((int)(0x80 | (code & 0x3F)));
    }
}

void execute(int cmd, stack *s, long size, int *dp, int *cc)
{
    long a, b;

    switch (cmd) {
    case CMD_PUSH:
        stack_push(s, size);
        break;

    case CMD_POP:
        if (s->n >= 1)
            stack_pop(s);
        break;

    case CMD_ADD:
    case CMD_SUBTRACT:
    case CMD_MULTIPLY:
    case CMD_DIVIDE:
    case CMD_MOD:
    case CMD_GREATER:
        if (s->n < 2)
            break;
        a = s->v[s->n - 1]; /* top */
        b = s->v[s->n - 2]; /* second */
        if ((cmd == CMD_DIVIDE || cmd == CMD_MOD) && a == 0)
            break;
        s->n -= 2;
        switch (cmd) {
        case CMD_ADD:      stack_push(s, wrap((unsigned long)b + (unsigned long)a)); break;
        case CMD_SUBTRACT: stack_push(s, wrap((unsigned long)b - (unsigned long)a)); break;
        case CMD_MULTIPLY: stack_push(s, wrap((unsigned long)b * (unsigned long)a)); break;
        case CMD_DIVIDE:   stack_push(s, divide(b, a)); break;
        case CMD_MOD:      stack_push(s, mod(b, a)); break;
        case CMD_GREATER:  stack_push(s, b > a); break;
        }
        break;

    case CMD_NOT:
        if (s->n >= 1)
            stack_push(s, stack_pop(s) == 0);
        break;

    case CMD_POINTER:
        if (s->n >= 1)
            *dp = (int)((*dp + stack_pop(s) % 4 + 4) % 4);
        break;

    case CMD_SWITCH:
        if (s->n >= 1 && stack_pop(s) % 2 != 0)
            *cc ^= 1;
        break;

    case CMD_DUPLICATE:
        if (s->n >= 1)
            stack_push(s, s->v[s->n - 1]);
        break;

    case CMD_ROLL:
        if (s->n < 2)
            break;
        a = s->v[s->n - 1]; /* rolls */
        b = s->v[s->n - 2]; /* depth */
        if (b < 0 || (size_t)b > s->n - 2)
            break;
        s->n -= 2;
        stack_roll(s, b, a);
        break;

    case CMD_IN_NUMBER:
        fflush(stdout);
        if (read_number(&a))
            stack_push(s, a);
        break;

    case CMD_IN_CHAR:
        fflush(stdout);
        if (read_char(&a))
            stack_push(s, a);
        break;

    case CMD_OUT_NUMBER:
        if (s->n >= 1)
            printf("%ld", stack_pop(s));
        break;

    case CMD_OUT_CHAR:
        if (s->n >= 1 && s->v[s->n - 1] >= 0 && s->v[s->n - 1] <= 0x10FFFF)
            write_char(stack_pop(s));
        break;
    }
}
