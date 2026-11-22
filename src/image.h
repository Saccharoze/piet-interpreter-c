#ifndef IMAGE_H
#define IMAGE_H

typedef unsigned char pixel[3];

typedef struct picture {
    int w;
    int h;
    pixel *grid; /* h rows of w pixels, top-left first */
} picture;

/* Reads a binary (P6) or plain (P3) PPM file.
   Prints an error and returns NULL if the file can't be read. */
picture *read_ppm(const char *path);

void free_picture(picture *img);

#endif
