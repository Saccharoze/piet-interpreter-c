#include <stdio.h>
#include <stdlib.h>
#include "blocks.h"
#include "colour.h"

static const int DX[4] = {1, 0, -1, 0};
static const int DY[4] = {0, 1, 0, -1};

/* How far (x, y) lies in direction d: the larger, the further. */
static int reach(int d, int x, int y)
{
    return d == RIGHT ? x : d == DOWN ? y : d == LEFT ? -x : -y;
}

static void *alloc(size_t n, size_t size)
{
    void *p = calloc(n ? n : 1, size);
    if (p == NULL) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    return p;
}

blocks *find_blocks(const picture *img)
{
    int w = img->w, h = img->h, n = w * h;
    blocks *b = alloc(1, sizeof *b);
    b->w = w;
    b->h = h;
    b->colour = alloc(n, 1);
    b->id = alloc(n, sizeof(int));
    for (int i = 0; i < n; i++) {
        b->colour[i] = (unsigned char)colour_of(img->grid[i]);
        b->id[i] = -1;
    }

    /* Flood fill with an explicit stack: big blocks would overflow the
       call stack with a recursive fill. */
    int *todo = alloc(n, sizeof(int));
    b->size = alloc(n, sizeof(int));
    b->block_colour = alloc(n, sizeof(int));
    for (int start = 0; start < n; start++) {
        int c = b->colour[start];
        if (c == WHITE || c == BLACK || b->id[start] >= 0)
            continue;
        int id = b->count++, top = 0;
        b->block_colour[id] = c;
        b->id[start] = id;
        todo[top++] = start;
        while (top > 0) {
            int i = todo[--top], x = i % w, y = i / w;
            b->size[id]++;
            for (int d = 0; d < 4; d++) {
                int nx = x + DX[d], ny = y + DY[d], j = ny * w + nx;
                if (nx < 0 || ny < 0 || nx >= w || ny >= h)
                    continue;
                if (b->id[j] < 0 && b->colour[j] == c) {
                    b->id[j] = id;
                    todo[top++] = j;
                }
            }
        }
    }
    free(todo);

    /* Exit codel: on the block's edge furthest along the DP, the codel
       furthest towards the CC side (CC left = DP turned anticlockwise). */
    b->exit = alloc((size_t)b->count * 8, sizeof(int));
    for (int i = 0; i < b->count * 8; i++)
        b->exit[i] = -1;
    for (int i = 0; i < n; i++) {
        int id = b->id[i], x = i % w, y = i / w;
        if (id < 0)
            continue;
        for (int dp = 0; dp < 4; dp++)
            for (int cc = 0; cc < 2; cc++) {
                int side = cc == CC_LEFT ? (dp + 3) % 4 : (dp + 1) % 4;
                int *e = &EXIT(b, id, dp, cc);
                if (*e < 0) {
                    *e = i;
                    continue;
                }
                int ex = *e % w, ey = *e / w;
                int r = reach(dp, x, y), er = reach(dp, ex, ey);
                if (r > er || (r == er && reach(side, x, y) > reach(side, ex, ey)))
                    *e = i;
            }
    }
    return b;
}

void free_blocks(blocks *b)
{
    if (b == NULL)
        return;
    free(b->colour);
    free(b->id);
    free(b->size);
    free(b->block_colour);
    free(b->exit);
    free(b);
}
