#ifndef BLOCKS_H
#define BLOCKS_H

#include "image.h"

/* Directions, clockwise: the direction pointer (DP) is one of these. */
enum { RIGHT, DOWN, LEFT, UP };

/* Codel chooser (CC): which end of the exit edge, seen from the DP. */
enum { CC_LEFT, CC_RIGHT };

typedef struct blocks {
    int w, h;
    unsigned char *colour; /* colour of each codel (see colour.h) */
    int *id;               /* block of each codel, -1 for white and black */
    int count;             /* number of colour blocks */
    int *size;             /* codels in each block */
    int *block_colour;     /* colour of each block */
    int *exit;             /* codel through which each block is left, for each DP and CC */
} blocks;

#define EXIT(b, block, dp, cc) ((b)->exit[(block) * 8 + (dp) * 2 + (cc)])

/* Groups the codels into colour blocks (touching codels of one colour)
   and finds the exit codel of every block for each DP and CC. */
blocks *find_blocks(const picture *img);

void free_blocks(blocks *b);

#endif
