#include <stdlib.h>
#include <string.h>
#include "stack.h"

void stack_push(stack *s, long x)
{
    if (s->n == s->cap) {
        size_t cap = s->cap ? s->cap * 2 : 64;
        long *v = realloc(s->v, cap * sizeof *v);
        if (v == NULL) {
            fprintf(stderr, "out of memory\n");
            exit(1);
        }
        s->v = v;
        s->cap = cap;
    }
    s->v[s->n++] = x;
}

long stack_pop(stack *s)
{
    return s->v[--s->n];
}

void stack_roll(stack *s, long depth, long rolls)
{
    if (depth <= 1)
        return;
    long r = rolls % depth;
    if (r < 0)
        r += depth;
    if (r == 0)
        return;

    /* The top `depth` values, bottom first: the value at i moves to i + r. */
    long *seg = s->v + s->n - depth;
    long *tmp = malloc((size_t)depth * sizeof *tmp);
    if (tmp == NULL) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    for (long i = 0; i < depth; i++)
        tmp[(i + r) % depth] = seg[i];
    memcpy(seg, tmp, (size_t)depth * sizeof *tmp);
    free(tmp);
}

void stack_print(const stack *s, FILE *f)
{
    fprintf(f, "[");
    for (size_t i = 0; i < s->n; i++)
        fprintf(f, i ? " %ld" : "%ld", s->v[i]);
    fprintf(f, "]");
}

void stack_free(stack *s)
{
    free(s->v);
    s->v = NULL;
    s->n = s->cap = 0;
}
