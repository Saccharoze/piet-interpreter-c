#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "blocks.h"
#include "colour.h"
#include "commands.h"
#include "image.h"
#include "stack.h"

static const int DX[4] = {1, 0, -1, 0};
static const int DY[4] = {0, 1, 0, -1};
static const char *DIRECTION[4] = {"right", "down", "left", "up"};

typedef struct state {
    const blocks *b;
    int dp, cc;
    stack s;
    int trace;
    long step;
    int *seen;      /* white codel x DP: last slide that went through it */
    int slide_mark;
} state;

/* The codel next to `i` in direction d, or -1 if it is off the image or black. */
static int neighbour(const blocks *b, int i, int d)
{
    int x = i % b->w + DX[d], y = i / b->w + DY[d];
    if (x < 0 || y < 0 || x >= b->w || y >= b->h)
        return -1;
    int j = y * b->w + x;
    return b->colour[j] == BLACK ? -1 : j;
}

/* Slides across white from codel i in a straight line. When it hits black
   or the edge, it toggles the CC and turns the DP clockwise. Returns the
   first coloured codel reached, or -1 if it would go round forever. */
static int slide(state *st, int i)
{
    const blocks *b = st->b;
    st->slide_mark++;
    for (;;) {
        int *seen = &st->seen[i * 4 + st->dp];
        if (*seen == st->slide_mark)
            return -1;
        *seen = st->slide_mark;
        int j = neighbour(b, i, st->dp);
        if (j < 0) {
            st->cc ^= 1;
            st->dp = (st->dp + 1) % 4;
        } else if (b->colour[j] == WHITE) {
            i = j;
        } else {
            return j;
        }
    }
}

static void trace(const state *st, int from, int to, int at, int cmd)
{
    const blocks *b = st->b;
    fprintf(stderr, "%ld: %s (%d) -> %s at (%d,%d), %s, now DP %s CC %s ",
            st->step, colour_name(b->block_colour[from]), b->size[from],
            colour_name(b->block_colour[to]), at % b->w, at / b->w, command_name(cmd),
            DIRECTION[st->dp], st->cc == CC_LEFT ? "left" : "right");
    stack_print(&st->s, stderr);
    fprintf(stderr, "\n");
}

static void run(state *st)
{
    const blocks *b = st->b;
    int cur;

    /* Start at the top-left codel, DP right, CC left. */
    if (b->colour[0] == BLACK)
        return;
    if (b->colour[0] == WHITE) {
        int j = slide(st, 0);
        if (j < 0)
            return;
        cur = b->id[j];
    } else {
        cur = b->id[0];
    }

    /* When the way out is blocked: toggle the CC, then turn the DP, in
       turn. Eight blocked tries in a row end the program. */
    int tries = 0;
    while (tries < 8) {
        int j = neighbour(b, EXIT(b, cur, st->dp, st->cc), st->dp);
        if (j < 0) {
            if (tries % 2 == 0)
                st->cc ^= 1;
            else
                st->dp = (st->dp + 1) % 4;
            tries++;
            continue;
        }
        tries = 0;
        if (b->colour[j] == WHITE) {
            j = slide(st, j);
            if (j < 0)
                return;
            st->step++;
            if (st->trace)
                trace(st, cur, b->id[j], j, CMD_NONE);
            cur = b->id[j];
            continue;
        }
        int next = b->id[j];
        int cmd = command_between(b->block_colour[cur], b->block_colour[next]);
        execute(cmd, &st->s, b->size[cur], &st->dp, &st->cc);
        st->step++;
        if (st->trace) {
            fflush(stdout);
            trace(st, cur, next, j, cmd);
        }
        cur = next;
    }
}

static void usage(void)
{
    fprintf(stderr, "usage: piet [-t] program.ppm\n"
                    "  -t  trace every step on stderr\n");
}

int main(int argc, char **argv)
{
    int trace_on = 0;
    const char *path = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0) {
            trace_on = 1;
        } else if (argv[i][0] == '-' || path != NULL) {
            usage();
            return 2;
        } else {
            path = argv[i];
        }
    }
    if (path == NULL) {
        usage();
        return 2;
    }

    picture *img = read_ppm(path);
    if (img == NULL)
        return 1;
    blocks *b = find_blocks(img);
    free_picture(img);

    state st = {0};
    st.b = b;
    st.dp = RIGHT;
    st.cc = CC_LEFT;
    st.trace = trace_on;
    st.seen = calloc((size_t)b->w * (size_t)b->h * 4, sizeof(int));
    if (st.seen == NULL) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }

    run(&st);

    fflush(stdout);
    free(st.seen);
    stack_free(&st.s);
    free_blocks(b);
    return 0;
}
