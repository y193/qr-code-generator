/*
 * Copyright (c) 2021 y193
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file module.c
 * @brief module implementation
 */
#include "module.h"

static const uint8_t alignment_pattern_gaps[] = {
    -1, -1, -1, -1, -1, -1, 16, 18, 20, 22, 24, 26, 28, 20,
    22, 24, 24, 26, 28, 28, 22, 24, 24, 26, 26, 28, 28, 24,
    24, 26, 26, 26, 28, 28, 24, 26, 26, 26, 28, 28};

static const uint32_t version_strings[] = {
    0x7C94,  0x85BC,  0x9A99,  0xA4D3,  0xBBF6,  0xC762,  0xD847,
    0xE60D,  0xF928,  0x10B78, 0x1145D, 0x12A17, 0x13532, 0x149A6,
    0x15683, 0x168C9, 0x177EC, 0x18EC4, 0x191E1, 0x1AFAB, 0x1B08E,
    0x1CC1A, 0x1D33F, 0x1ED75, 0x1F250, 0x209D5, 0x216F0, 0x228BA,
    0x2379F, 0x24B0B, 0x2542E, 0x26A64, 0x27541, 0x28C69};

/**
 * Returns the Nth bit of bit string.
 *
 * @param b bit string.
 * @param n index of bit string.
 * @return Nth bit of bit string.
 */
static int nth_bit(const uint8_t b[], int n) {
    return b[n / 8] >> (7 - n % 8) & 1;
}

/**
 * Place a horizontal line in the matrix.
 *
 * @param n matrix length.
 * @param m matrix.
 * @param y Y coordinate.
 * @param x leftmost X coordinate.
 * @param l horizontal line length.
 * @param t module type.
 */
static void add_horizontal_line(int n, module_type_t m[][n], int y, int x,
                                int l, module_type_t t) {
    for (int i = 0; i < l; i++)
        m[y][x + i] = t;
}

/**
 * Place a vertical line in the matrix.
 *
 * @param n matrix length.
 * @param m matrix.
 * @param y topmost Y coordinate.
 * @param x X coordinate.
 * @param l vertical line length.
 * @param t module type.
 */
static void add_vertical_line(int n, module_type_t m[][n], int y, int x, int l,
                              module_type_t t) {
    for (int i = 0; i < l; i++)
        m[y + i][x] = t;
}

/**
 * Place a filled rectangle in the matrix.
 *
 * @param n matrix length.
 * @param m matrix.
 * @param y upper left Y coordinate.
 * @param x upper left X coordinate.
 * @param h rectangle height.
 * @param w rectangle width.
 * @param t module type.
 */
static void add_filled_rectangle(int n, module_type_t m[][n], int y, int x,
                                 int h, int w, module_type_t t) {
    for (int i = 0; i < h; i++)
        for (int j = 0; j < w; j++)
            m[y + i][x + j] = t;
}

/**
 * Place a unfilled rectangle in the matrix.
 *
 * @param n matrix length.
 * @param m matrix.
 * @param y upper left Y coordinate.
 * @param x upper left X coordinate.
 * @param h rectangle height.
 * @param w rectangle width.
 * @param t module type.
 */
static void add_unfilled_rectangle(int n, module_type_t m[][n], int y, int x,
                                   int h, int w, module_type_t t) {
    add_horizontal_line(n, m, y, x, w, t);
    add_vertical_line(n, m, y + 1, x, h - 2, t);
    add_vertical_line(n, m, y + 1, x + w - 1, h - 2, t);
    add_horizontal_line(n, m, y + h - 1, x, w, t);
}

/**
 * Place one finder pattern in the matrix.
 *
 * @param n matrix length.
 * @param m matrix.
 * @param y upper left Y coordinate.
 * @param x upper left X coordinate.
 */
static void add_finder_pattern(int n, module_type_t m[][n], int y, int x) {
    add_filled_rectangle(n, m, y, x, 7, 7, MODULE_TYPE_DARK);
    add_unfilled_rectangle(n, m, y + 1, x + 1, 5, 5, MODULE_TYPE_LIGHT);
}

/**
 * Place finder patterns in the matrix.
 *
 * @param n matrix length.
 * @param m matrix.
 */
static void add_finder_patterns(int n, module_type_t m[][n]) {
    add_finder_pattern(n, m, 0, 0);
    add_finder_pattern(n, m, 0, n - 7);
    add_finder_pattern(n, m, n - 7, 0);
}

/**
 * Place separators in the matrix.
 *
 * @param n matrix length.
 * @param m matrix.
 */
static void add_separators(int n, module_type_t m[][n]) {
    add_vertical_line(n, m, 0, 7, 7, MODULE_TYPE_LIGHT);
    add_vertical_line(n, m, 0, n - 8, 7, MODULE_TYPE_LIGHT);
    add_horizontal_line(n, m, 7, 0, 8, MODULE_TYPE_LIGHT);
    add_horizontal_line(n, m, 7, n - 8, 8, MODULE_TYPE_LIGHT);
    add_vertical_line(n, m, n - 7, 7, 7, MODULE_TYPE_LIGHT);
    add_horizontal_line(n, m, n - 8, 0, 8, MODULE_TYPE_LIGHT);
}

/**
 * Place one alignment pattern in the matrix.
 *
 * @param n matrix length.
 * @param m matrix.
 * @param y upper left Y coordinate.
 * @param x upper left X coordinate.
 */
static void add_alignment_pattern(int n, module_type_t m[][n], int y, int x) {
    add_filled_rectangle(n, m, y, x, 5, 5, MODULE_TYPE_DARK);
    add_unfilled_rectangle(n, m, y + 1, x + 1, 3, 3, MODULE_TYPE_LIGHT);
}

/**
 * Place alignment patterns in the matrix.
 *
 * @param n matrix length.
 * @param m matrix.
 * @param v version.
 */
static void add_alignment_patterns(int n, module_type_t m[][n], int v) {
    add_alignment_pattern(n, m, n - 9, n - 9);

    if (v < 6)
        return;

    int g = alignment_pattern_gaps[v];

    for (int i = n - 9;; i = i - g > 6 ? i - g : 4) {
        for (int j = n - 9;; j = j - g > 6 ? j - g : 4) {
            if ((i != 4 && i != n - 9) || (j != 4 && j != n - 9))
                add_alignment_pattern(n, m, i, j);

            if (j == 4)
                break;
        }

        if (i == 4)
            break;
    }
}

/**
 * Place timing patterns in the matrix.
 *
 * @param n matrix length.
 * @param m matrix.
 */
static void add_timing_patterns(int n, module_type_t m[][n]) {
    for (int i = 0; i < n - 16; i++)
        m[6][8 + i] = i % 2 == 0 ? MODULE_TYPE_DARK : MODULE_TYPE_LIGHT;

    for (int i = 0; i < n - 16; i++)
        m[8 + i][6] = i % 2 == 0 ? MODULE_TYPE_DARK : MODULE_TYPE_LIGHT;
}

/**
 * Place a dark module at position (4V + 9, 8) in the matrix.
 *
 * @param n matrix length.
 * @param m matrix.
 */
static void add_dark_module(int n, module_type_t m[][n]) {
    m[n - 8][8] = MODULE_TYPE_DARK;
}

/**
 * Reserve the format information area in the matrix.
 *
 * @param n matrix length.
 * @param m matrix.
 */
static void reserve_format_info(int n, module_type_t m[][n]) {
    add_vertical_line(n, m, 0, 8, 8, MODULE_TYPE_RESERVED);
    add_horizontal_line(n, m, 8, 0, 9, MODULE_TYPE_RESERVED);
    add_horizontal_line(n, m, 8, n - 8, 8, MODULE_TYPE_RESERVED);
    add_vertical_line(n, m, n - 7, 8, 7, MODULE_TYPE_RESERVED);
}

/**
 * Place version information in the matrix.
 *
 * @param n matrix length.
 * @param m matrix.
 * @param v version.
 */
static void add_version_info(int n, module_type_t m[][n], int v) {
    for (int i = 0; i < 6; i++)
        for (int j = 0; j < 3; j++)
            m[i][n - 11 + j] = version_strings[v - 6] >> (i * 3 + j) & 1;

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 6; j++)
            m[n - 11 + i][j] = version_strings[v - 6] >> (i + j * 3) & 1;
}

/**
 * Place the data bits in the matrix.
 *
 * @param n matrix length.
 * @param m matrix.
 * @param b data bits.
 */
static void place_data_bits(int n, module_type_t m[][n], const uint8_t b[]) {
    int y = n - 1;
    int v = -1;
    int k = 0;

    for (int x = n - 1; x > 0; x -= 2 + (x == 8)) {
        for (int i = 0; i < n; i++)
            for (int j = 0; j < 2; j++)
                if (m[y + v * i][x - j] == MODULE_TYPE_NONE)
                    m[y + v * i][x - j] = nth_bit(b, k++);

        y ^= n - 1;
        v = -v;
    }
}

/**
 * Returns matrix length by number of modules.
 *
 * @param v version.
 * @return matrix length by number of modules.
 */
int matrix_length(int v) {
    return v * 4 + 21;
}

/**
 * Place modules in matrix.
 *
 * @param n matrix length.
 * @param m matrix.
 * @param f true if modules to be applied masking and false otherwise.
 * @param b final message.
 * @param v version.
 */
void place_modules(int n, module_type_t m[][n], bool f[][n], const uint8_t b[],
                   int v) {
    add_filled_rectangle(n, m, 0, 0, n, n, MODULE_TYPE_NONE);

    add_finder_patterns(n, m);
    add_separators(n, m);

    if (v >= 1)
        add_alignment_patterns(n, m, v);

    reserve_format_info(n, m);
    add_timing_patterns(n, m);
    add_dark_module(n, m);

    if (v >= 6)
        add_version_info(n, m, v);

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            f[i][j] = m[i][j] == MODULE_TYPE_NONE;

    place_data_bits(n, m, b);
}
