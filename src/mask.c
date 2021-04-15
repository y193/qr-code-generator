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
 * @file mask.c
 * @brief mask implementation
 */
#include <stdint.h>
#include <stdlib.h>
#include "mask.h"

#define PENALTY_WEIGHT_N1 3
#define PENALTY_WEIGHT_N2 3
#define PENALTY_WEIGHT_N3 40
#define PENALTY_WEIGHT_N4 10

static const uint16_t format_strings[][8] = {
    {0x77C4, 0x72F3, 0x7DAA, 0x789D, 0x662F, 0x6318, 0x6C41, 0x6976},
    {0x5412, 0x5125, 0x5E7C, 0x5B4B, 0x45F9, 0x40CE, 0x4F97, 0x4AA0},
    {0x355F, 0x3068, 0x3F31, 0x3A06, 0x24B4, 0x2183, 0x2EDA, 0x2BED},
    {0x1689, 0x13BE, 0x1CE7, 0x19D0, 0x0762, 0x0255, 0x0D0C, 0x083B}};

/**
 * Returns mask pattern 0 generation conditions.
 *
 * @param y Y coordinate.
 * @param x X coordinate.
 * @return mask pattern 0 generation conditions.
 */
static bool mask_pattern0(int y, int x) {
    return (y + x) % 2 == 0;
}

/**
 * Returns mask pattern 1 generation conditions.
 *
 * @param y Y coordinate.
 * @return mask pattern 1 generation conditions.
 */
static bool mask_pattern1(int y) {
    return y % 2 == 0;
}

/**
 * Returns mask pattern 2 generation conditions.
 *
 * @param x X coordinate.
 * @return mask pattern 2 generation conditions.
 */
static bool mask_pattern2(int x) {
    return x % 3 == 0;
}

/**
 * Returns mask pattern 3 generation conditions.
 *
 * @param y Y coordinate.
 * @param x X coordinate.
 * @return mask pattern 3 generation conditions.
 */
static bool mask_pattern3(int y, int x) {
    return (y + x) % 3 == 0;
}

/**
 * Returns mask pattern 4 generation conditions.
 *
 * @param y Y coordinate.
 * @param x X coordinate.
 * @return mask pattern 4 generation conditions.
 */
static bool mask_pattern4(int y, int x) {
    return ((y / 2) + (x / 3)) % 2 == 0;
}

/**
 * Returns mask pattern 5 generation conditions.
 *
 * @param y Y coordinate.
 * @param x X coordinate.
 * @return mask pattern 5 generation conditions.
 */
static bool mask_pattern5(int y, int x) {
    return (y * x) % 2 + (y * x) % 3 == 0;
}

/**
 * Returns mask pattern 6 generation conditions.
 *
 * @param y Y coordinate.
 * @param x X coordinate.
 * @return mask pattern 6 generation conditions.
 */
static bool mask_pattern6(int y, int x) {
    return ((y * x) % 2 + (y * x) % 3) % 2 == 0;
}

/**
 * Returns mask pattern 7 generation conditions.
 *
 * @param y Y coordinate.
 * @param x X coordinate.
 * @return mask pattern 7 generation conditions.
 */
static bool mask_pattern7(int y, int x) {
    return ((y * x) % 3 + (y + x) % 2) % 2 == 0;
}

/**
 * Place format information in the matrix.
 *
 * @param n matrix length.
 * @param m matrix.
 * @param e error correction level.
 * @param p mask pattern.
 */
static void add_format_info(int n, module_type_t m[][n],
                            error_correction_level_t e, int p) {
    uint16_t f = format_strings[e][p];

    for (int i = 0; i < 8; i++)
        m[i + (i >= 6)][8] = (f >> i) & 1;

    for (int i = 8; i < 15; i++)
        m[n + i - 15][8] = (f >> i) & 1;

    for (int i = 0; i < 7; i++)
        m[8][i + (i >= 6)] = (f >> (14 - i)) & 1;

    for (int i = 7; i < 15; i++)
        m[8][n + i - 15] = (f >> (14 - i)) & 1;
}

/**
 * Returns the penalty score under evaluation condition 1.
 *
 * @param n matrix length.
 * @param m matrix.
 * @return penalty score.
 */
static int eval_penalty1(int n, const module_type_t m[][n]) {
    int s = 0;

    for (int k = 0; k < 2; k++) {
        for (int i = 0; i < n; i++) {
            module_type_t u = MODULE_TYPE_NONE;
            int l = 0;

            for (int j = 0; j < n; j++) {
                module_type_t t = m[i * (1 - k) + j * k][i * k + j * (1 - k)];

                if (t == u) {
                    l++;

                } else {
                    if (l >= 5)
                        s += PENALTY_WEIGHT_N1 + (l - 5);

                    l = 1;
                    u = t;
                }
            }

            if (l >= 5)
                s += PENALTY_WEIGHT_N1 + (l - 5);
        }
    }

    return s;
}

/**
 * Returns the penalty score under evaluation condition 2.
 *
 * @param n matrix length.
 * @param m matrix.
 * @return penalty score.
 */
static int eval_penalty2(int n, const module_type_t m[][n]) {
    int s = 0;

    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - 1; j++)
            if (m[i][j] == m[i][j + 1] && m[i][j] == m[i + 1][j] &&
                m[i][j] == m[i + 1][j + 1])
                s += PENALTY_WEIGHT_N2;

    return s;
}

/**
 * Returns the penalty score under evaluation condition 3.
 *
 * @param n matrix length.
 * @param m matrix.
 * @return penalty score.
 */
static int eval_penalty3(int n, const module_type_t m[][n]) {
    int s = 0;

    for (int k = 0; k < 2; k++) {
        for (int i = 0; i < n; i++) {
            uint16_t p = 0;

            for (int j = 0; j < n; j++) {
                p <<= 1;
                p |= m[i * (1 - k) + j * k][i * k + j * (1 - k)];
                p &= 0x7FF;

                if (j >= 10 && (p == 0x5D0 || p == 0x5D))
                    s += PENALTY_WEIGHT_N3;
            }
        }
    }

    return s;
}

/**
 * Returns the penalty score under evaluation condition 4.
 *
 * @param n matrix length.
 * @param m matrix.
 * @return penalty score.
 */
static int eval_penalty4(int n, const module_type_t m[][n]) {
    int d = 0;

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (m[i][j] == MODULE_TYPE_DARK)
                d++;

    return abs(d * 2 - (n * n)) * 10 / (n * n) * PENALTY_WEIGHT_N4;
}

/**
 * Mask with the specified mask pattern.
 *
 * @param n matrix length.
 * @param d output matrix to be masked.
 * @param s input matrix.
 * @param f true if modules to be applied masking and false otherwise.
 * @param e error correction level.
 * @param p mask pattern.
 */
void mask_modules(int n, module_type_t d[][n], const module_type_t s[][n],
                  const bool f[][n], error_correction_level_t e, int p) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (f[i][j])
                switch (p) {
                case 0:
                    d[i][j] = s[i][j] ^ mask_pattern0(i, j);
                    break;

                case 1:
                    d[i][j] = s[i][j] ^ mask_pattern1(i);
                    break;

                case 2:
                    d[i][j] = s[i][j] ^ mask_pattern2(j);
                    break;

                case 3:
                    d[i][j] = s[i][j] ^ mask_pattern3(i, j);
                    break;

                case 4:
                    d[i][j] = s[i][j] ^ mask_pattern4(i, j);
                    break;

                case 5:
                    d[i][j] = s[i][j] ^ mask_pattern5(i, j);
                    break;

                case 6:
                    d[i][j] = s[i][j] ^ mask_pattern6(i, j);
                    break;

                case 7:
                    d[i][j] = s[i][j] ^ mask_pattern7(i, j);
                    break;
                }

            else
                d[i][j] = s[i][j];

    add_format_info(n, d, e, p);
}

/**
 * Returns the penalty score based on the evaluation conditions defined in the
 * QR code specification.
 *
 * @param n matrix length.
 * @param m matrix.
 * @return penalty score.
 */
int eval_penalty(int n, const module_type_t m[][n]) {
    return eval_penalty1(n, m) + eval_penalty2(n, m) + eval_penalty3(n, m) +
           eval_penalty4(n, m);
}

/**
 * Mask using the mask pattern with the lowest penalty.
 *
 * @param n matrix length.
 * @param d output matrix to be masked.
 * @param s input matrix.
 * @param f true if modules to be applied masking and false otherwise.
 * @param e error correction level.
 */
void mask_modules_auto(int n, module_type_t d[][n], const module_type_t s[][n],
                       const bool f[][n], error_correction_level_t e) {
    module_type_t t[n][n];
    int z[8];

    for (int i = 0; i < 8; i++) {
        mask_modules(n, t, s, f, e, i);
        z[i] = eval_penalty(n, t);
    }

    int l = z[0];
    int p = 0;

    for (int i = 1; i < 8; i++)
        if (z[i] < l) {
            l = z[i];
            p = i;
        }

    mask_modules(n, d, s, f, e, p);
}
