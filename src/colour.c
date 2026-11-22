#include "colour.h"

/* Channels that are on for each hue: red, green, blue. */
static const int hue_channels[6][3] = {
    {1, 0, 0}, /* red */
    {1, 1, 0}, /* yellow */
    {0, 1, 0}, /* green */
    {0, 1, 1}, /* cyan */
    {0, 0, 1}, /* blue */
    {1, 0, 1}, /* magenta */
};

/* Light colours raise the off channels to `mid`, dark colours lower the on
   channels to `mid`. The standard palette uses 192 and 0; the variants
   with 128 for `mid` and 32 for `off` are accepted too. */
static const int mids[2] = {192, 128};
static const int offs[2] = {0, 32};

static int channel(int on, int lightness, int mid, int off)
{
    if (on)
        return lightness == 2 ? mid : 255;
    return lightness == 0 ? mid : off;
}

int colour_of(const pixel p)
{
    for (int m = 0; m < 2; m++)
        for (int o = 0; o < 2; o++)
            for (int hue = 0; hue < 6; hue++)
                for (int light = 0; light < 3; light++) {
                    int k = 0;
                    while (k < 3 && p[k] == channel(hue_channels[hue][k], light, mids[m], offs[o]))
                        k++;
                    if (k == 3)
                        return hue * 3 + light;
                }

    if (0.202 * p[0] + 0.707 * p[1] + 0.071 * p[2] < 128)
        return BLACK;
    return WHITE;
}

const char *colour_name(int c)
{
    static const char *names[20] = {
        "light red", "red", "dark red",
        "light yellow", "yellow", "dark yellow",
        "light green", "green", "dark green",
        "light cyan", "cyan", "dark cyan",
        "light blue", "blue", "dark blue",
        "light magenta", "magenta", "dark magenta",
        "white", "black",
    };
    return c >= 0 && c < 20 ? names[c] : "?";
}
