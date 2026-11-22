#!/usr/bin/env python3
"""Writes small Piet test programs into a folder: name.ppm, the expected
output name.out and, when the program reads, its input name.in.

Most programs are one row of colour blocks read left to right, ending in a
block with no way out. A push block is as wide as the number it pushes."""

import os
import sys

COMMANDS = ['none', 'push', 'pop', 'add', 'subtract', 'multiply', 'divide', 'mod', 'not',
            'greater', 'pointer', 'switch', 'duplicate', 'roll', 'in_number', 'in_char',
            'out_number', 'out_char']

HUES = [(1, 0, 0), (1, 1, 0), (0, 1, 0), (0, 1, 1), (0, 0, 1), (1, 0, 1)]

WHITE, BLACK = 'white', 'black'


def rgb(colour, mid, off):
    """Colours 0-17 are hue * 3 + lightness (light, normal, dark)."""
    if colour == WHITE:
        return (255, 255, 255)
    if colour == BLACK:
        return (0, 0, 0)
    if isinstance(colour, tuple):
        return colour
    hue, light = divmod(colour, 3)
    return tuple((mid if light == 2 else 255) if on else (mid if light == 0 else off)
                 for on in HUES[hue])


def after(colour, command):
    """The colour that runs `command` when entered from `colour`."""
    k = COMMANDS.index(command)
    hue, light = divmod(colour, 3)
    return (hue + k // 3) % 6 * 3 + (light + k % 3) % 3


def line(ops, start=0, gap=WHITE):
    """One row of blocks for ops like 'push 72', 'out_char', 'white 3'.
    Returns the row and the colour the last op leads to."""
    row, colour = [], start
    for op in ops:
        name, *arg = op.split()
        if name == 'white':
            row += [colour] + [gap] * int(arg[0])
            # No command runs across white: the block after it gets the colour
            # that would push, so a wrongly run command changes the output.
            colour = after(colour, 'push')
            continue
        row += [colour] * (int(arg[0]) if name == 'push' else 1)
        colour = after(colour, name)
    return row, colour


def straight(ops, gap=WHITE, fill=BLACK):
    """A row of blocks, then a last block shaped so that every way out is
    blocked: the program stops there."""
    row, last = line(ops, gap=gap)
    k = len(row)
    return [row + [last],
            [fill] * k + [last],
            [fill] * (k - 1) + [last, last]]


def pointer_test():
    """push 7, push 1, pointer: the DP turns down. Below the next block D,
    out(number) prints 7. Had the DP stayed right, out(char) would run."""
    row, d = line(['push 7', 'push 1', 'pointer'])
    k = len(row)
    t, x = after(d, 'out_number'), after(d, 'out_char')
    return [row + [d, x, t],
            [BLACK] * k + [t, t, t]]


def switch_test():
    """push 7, push 1, switch: the CC turns right, so the tall block S is
    left through its bottom codel, into out(number). With the CC left, the
    top codel would lead to out(char)."""
    row, s = line(['push 7', 'push 1', 'switch'])
    k = len(row)
    y, z = after(s, 'out_number'), after(s, 'out_char')
    grid = [row + [s, z]] + [[BLACK] * (k + 2) for _ in range(4)]
    grid[1][k] = grid[2][k] = s
    for r, c in [(2, k + 1), (3, k + 1), (4, k + 1), (4, k)]:
        grid[r][c] = y
    return grid


SPACE = ['push 32', 'out_char']
NEWLINE = ['push 10', 'out_char']
HI = ['push 72', 'out_char', 'push 105', 'out_char'] + NEWLINE

# Off-palette colours: light ones count as white, dark ones as black.
PALE = [(230, 200, 240), (200, 230, 210), (250, 250, 180), (180, 210, 250)]
DARK = (60, 40, 70)

TESTS = {
    'hello': dict(grid=straight(HI), out='Hi\n'),
    'hello-standard-palette': dict(grid=straight(HI), mid=192, comment=True, out='Hi\n'),
    'hello-off-32': dict(grid=straight(HI), off=32, out='Hi\n'),
    'hello-plain-ppm': dict(grid=straight(HI), plain=True, comment=True, out='Hi\n'),
    'arithmetic': dict(grid=straight(
        ['push 7', 'push 3', 'add', 'out_number'] + SPACE +                          # 10
        ['push 3', 'push 7', 'subtract', 'out_number'] + SPACE +                     # -4
        ['push 6', 'push 7', 'multiply', 'out_number'] + SPACE +                     # 42
        ['push 7', 'push 2', 'divide', 'out_number'] + SPACE +                       # 3
        ['push 1', 'push 8', 'subtract', 'push 2', 'divide', 'out_number'] + SPACE + # -7/2 = -4
        ['push 1', 'push 8', 'subtract', 'push 3', 'mod', 'out_number'] + SPACE +    # -7 mod 3 = 2
        ['push 7', 'push 1', 'push 4', 'subtract', 'mod', 'out_number'] + SPACE +    # 7 mod -3 = -2
        ['push 5', 'push 3', 'greater', 'out_number'] + SPACE +                      # 1
        ['push 3', 'push 5', 'greater', 'out_number'] + SPACE +                      # 0
        ['push 5', 'not', 'out_number'] + SPACE +                                    # 0
        ['push 1', 'push 1', 'subtract', 'not', 'out_number'] + NEWLINE),            # 1
        out='10 -4 42 3 -4 2 -2 1 0 0 1\n'),
    'stack': dict(grid=straight(
        ['add', 'pop'] +                                    # empty stack: nothing happens
        ['push 1', 'push 2', 'push 3', 'push 3', 'push 1', 'roll'] +
        ['out_number'] * 3 + SPACE +                        # [3 1 2]: 213
        ['push 4', 'duplicate', 'out_number', 'out_number'] + SPACE +
        ['push 5', 'push 6', 'pop', 'out_number'] + SPACE +
        ['push 1', 'push 2', 'push 3', 'push 3', 'push 1', 'push 2', 'subtract', 'roll'] +
        ['out_number'] * 3 + SPACE +                        # roll by -1, [2 3 1]: 132
        ['push 5', 'push 1', 'push 1', 'subtract', 'divide'] +
        ['out_number'] * 2 + NEWLINE),                      # division by 0 skipped: 05
        out='213 44 5 132 05\n'),
    'input': dict(grid=straight(
        ['in_number', 'in_number', 'add', 'out_number'] + NEWLINE +
        ['in_char', 'out_char'] * 3 +
        ['in_char', 'out_number']),                         # end of input: both skipped
        input='12 30xé!', out='42\nxé!'),
    'white': dict(grid=straight(['push 5', 'white 4', 'out_number'] + NEWLINE), out='5\n'),
    'off-palette': dict(grid=straight(['push 5', 'white 4', 'out_number'] + NEWLINE,
                                      gap=PALE, fill=DARK), out='5\n'),
    'pointer': dict(grid=pointer_test(), out='7'),
    'switch': dict(grid=switch_test(), out='7'),
}


def write_ppm(path, grid, mid=128, off=0, plain=False, comment=False):
    h, w = len(grid), len(grid[0])
    pixels, n = [], 0
    for r in grid:
        for c in r:
            if isinstance(c, list):  # a list of colours: take them in turn
                c = c[n % len(c)]
                n += 1
            pixels.append(rgb(c, mid, off))
    header = 'P3\n' if plain else 'P6\n'
    if comment:
        header += '# test program\n'
    header += f'{w} {h}\n255\n'
    with open(path, 'wb') as f:
        f.write(header.encode())
        if plain:
            f.write('\n'.join(' '.join(map(str, p)) for p in pixels).encode() + b'\n')
        else:
            f.write(bytes(v for p in pixels for v in p))


def main():
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    for name, t in TESTS.items():
        base = os.path.join(out, name)
        write_ppm(base + '.ppm', t['grid'], t.get('mid', 128), t.get('off', 0),
                  t.get('plain', False), t.get('comment', False))
        with open(base + '.out', 'w', encoding='utf-8') as f:
            f.write(t['out'])
        if 'input' in t:
            with open(base + '.in', 'w', encoding='utf-8') as f:
                f.write(t['input'])


if __name__ == '__main__':
    main()
