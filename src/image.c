#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "image.h"

/* Next number of the header (or of P3 pixel data), skipping blanks and
   '#' comments. Consumes the one blank that ends the number: in a P6 file
   the pixel bytes start right after it. Returns -1 on error. */
static long next_number(FILE *f)
{
    int c = fgetc(f);
    for (;;) {
        while (c != EOF && isspace(c))
            c = fgetc(f);
        if (c != '#')
            break;
        while (c != EOF && c != '\n')
            c = fgetc(f);
    }
    if (c == EOF || !isdigit(c))
        return -1;
    long n = 0;
    while (c != EOF && isdigit(c)) {
        if (n > 100000000L)
            return -1;
        n = n * 10 + (c - '0');
        c = fgetc(f);
    }
    if (c != EOF && !isspace(c))
        return -1;
    return n;
}

picture *read_ppm(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        perror(path);
        return NULL;
    }

    picture *img = NULL;
    int p = fgetc(f), kind = fgetc(f);
    if (p != 'P' || (kind != '6' && kind != '3')) {
        fprintf(stderr, "%s: not a PPM file (P6 or P3)\n", path);
        goto fail;
    }

    long w = next_number(f), h = next_number(f), maxval = next_number(f);
    if (w <= 0 || h <= 0 || maxval <= 0 || maxval > 255 || w * h > 100000000L) {
        fprintf(stderr, "%s: unsupported PPM header\n", path);
        goto fail;
    }

    size_t n = (size_t)w * (size_t)h;
    img = malloc(sizeof *img);
    if (img == NULL || (img->grid = malloc(n * sizeof(pixel))) == NULL) {
        fprintf(stderr, "%s: out of memory\n", path);
        free(img);
        img = NULL;
        goto fail;
    }
    img->w = (int)w;
    img->h = (int)h;

    if (kind == '6') {
        if (fread(img->grid, sizeof(pixel), n, f) != n) {
            fprintf(stderr, "%s: pixel data is cut short\n", path);
            goto fail;
        }
    } else {
        for (size_t i = 0; i < 3 * n; i++) {
            long v = next_number(f);
            if (v < 0 || v > maxval) {
                fprintf(stderr, "%s: bad pixel value\n", path);
                goto fail;
            }
            img->grid[i / 3][i % 3] = (unsigned char)v;
        }
    }

    if (maxval != 255)
        for (size_t i = 0; i < n; i++)
            for (int k = 0; k < 3; k++)
                img->grid[i][k] = (unsigned char)((img->grid[i][k] * 255 + maxval / 2) / maxval);

    fclose(f);
    return img;

fail:
    free_picture(img);
    fclose(f);
    return NULL;
}

void free_picture(picture *img)
{
    if (img == NULL)
        return;
    free(img->grid);
    free(img);
}
