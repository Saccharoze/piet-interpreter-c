#ifndef COLOUR_H
#define COLOUR_H

#include "image.h"

/* Colours 0 to 17 are hue * 3 + lightness:
   hue 0 red, 1 yellow, 2 green, 3 cyan, 4 blue, 5 magenta;
   lightness 0 light, 1 normal, 2 dark. */
#define WHITE 18
#define BLACK 19

#define HUE(c) ((c) / 3)
#define LIGHTNESS(c) ((c) % 3)

/* The Piet colour of a pixel. Any other colour counts as black or white,
   depending on its brightness. */
int colour_of(const pixel p);

const char *colour_name(int c);

#endif
