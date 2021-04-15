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
 * @file message.c
 * @brief message implementation
 */
#include "message.h"

static const int16_t num_codewords[] = {
    26,   44,   70,   100,  134,  172,  196,  242,  292,  346,
    404,  466,  532,  581,  655,  733,  815,  901,  991,  1085,
    1156, 1258, 1364, 1474, 1588, 1706, 1828, 1921, 2051, 2185,
    2323, 2465, 2611, 2761, 2876, 3034, 3196, 3362, 3532, 3706};

static const int8_t rs_block_table[][4][3] = {
    {{1, 19, 0}, {1, 16, 0}, {1, 13, 0}, {1, 9, 0}},
    {{1, 34, 0}, {1, 28, 0}, {1, 22, 0}, {1, 16, 0}},
    {{1, 55, 0}, {1, 44, 0}, {2, 17, 0}, {2, 13, 0}},
    {{1, 80, 0}, {2, 32, 0}, {2, 24, 0}, {4, 9, 0}},
    {{1, 108, 0}, {2, 43, 0}, {2, 15, 2}, {2, 11, 2}},
    {{2, 68, 0}, {4, 27, 0}, {4, 19, 0}, {4, 15, 0}},
    {{2, 78, 0}, {4, 31, 0}, {2, 14, 4}, {4, 13, 1}},
    {{2, 97, 0}, {2, 38, 2}, {4, 18, 2}, {4, 14, 2}},
    {{2, 116, 0}, {3, 36, 2}, {4, 16, 4}, {4, 12, 4}},
    {{2, 68, 2}, {4, 43, 1}, {6, 19, 2}, {6, 15, 2}},
    {{4, 81, 0}, {1, 50, 4}, {4, 22, 4}, {3, 12, 8}},
    {{2, 92, 2}, {6, 36, 2}, {4, 20, 6}, {7, 14, 4}},
    {{4, 107, 0}, {8, 37, 1}, {8, 20, 4}, {12, 11, 4}},
    {{3, 115, 1}, {4, 40, 5}, {11, 16, 5}, {11, 12, 5}},
    {{5, 87, 1}, {5, 41, 5}, {5, 24, 7}, {11, 12, 7}},
    {{5, 98, 1}, {7, 45, 3}, {15, 19, 2}, {3, 15, 13}},
    {{1, 107, 5}, {10, 46, 1}, {1, 22, 15}, {2, 14, 17}},
    {{5, 120, 1}, {9, 43, 4}, {17, 22, 1}, {2, 14, 19}},
    {{3, 113, 4}, {3, 44, 11}, {17, 21, 4}, {9, 13, 16}},
    {{3, 107, 5}, {3, 41, 13}, {15, 24, 5}, {15, 15, 10}},
    {{4, 116, 4}, {17, 42, 0}, {17, 22, 6}, {19, 16, 6}},
    {{2, 111, 7}, {17, 46, 0}, {7, 24, 16}, {34, 13, 0}},
    {{4, 121, 5}, {4, 47, 14}, {11, 24, 14}, {16, 15, 14}},
    {{6, 117, 4}, {6, 45, 14}, {11, 24, 16}, {30, 16, 2}},
    {{8, 106, 4}, {8, 47, 13}, {7, 24, 22}, {22, 15, 13}},
    {{10, 114, 2}, {19, 46, 4}, {28, 22, 6}, {33, 16, 4}},
    {{8, 122, 4}, {22, 45, 3}, {8, 23, 26}, {12, 15, 28}},
    {{3, 117, 10}, {3, 45, 23}, {4, 24, 31}, {11, 15, 31}},
    {{7, 116, 7}, {21, 45, 7}, {1, 23, 37}, {19, 15, 26}},
    {{5, 115, 10}, {19, 47, 10}, {15, 24, 25}, {23, 15, 25}},
    {{13, 115, 3}, {2, 46, 29}, {42, 24, 1}, {23, 15, 28}},
    {{17, 115, 0}, {10, 46, 23}, {10, 24, 35}, {19, 15, 35}},
    {{17, 115, 1}, {14, 46, 21}, {29, 24, 19}, {11, 15, 46}},
    {{13, 115, 6}, {14, 46, 23}, {44, 24, 7}, {59, 16, 1}},
    {{12, 121, 7}, {12, 47, 26}, {39, 24, 14}, {22, 15, 41}},
    {{6, 121, 14}, {6, 47, 34}, {46, 24, 10}, {2, 15, 64}},
    {{17, 122, 4}, {29, 46, 14}, {49, 24, 10}, {24, 15, 46}},
    {{4, 122, 18}, {13, 46, 32}, {48, 24, 14}, {42, 15, 32}},
    {{20, 117, 4}, {40, 47, 7}, {43, 24, 22}, {10, 15, 67}},
    {{19, 118, 6}, {18, 47, 31}, {34, 24, 34}, {20, 15, 61}}};

/**
 * Interleave the codewords.
 *
 * @param l codewords length.
 * @param c codewords.
 * @param m number of blocks in group 1.
 * @param n number of blocks in group 2.
 * @param k number of codewords in each block of group 1.
 * @param p pointer to column 1 of each block.
 */
static void interleave_codewords(int l, uint8_t c[], int m, int n, int k,
                                 const uint8_t *const p[]) {
    uint8_t t[l];
    int x = 0;

    for (int i = 0; i < k; i++)
        for (int j = 0; j < m + n; j++)
            t[x++] = p[j][i];

    for (int i = 0; i < n; i++)
        t[x++] = p[m + i][k];

    for (int i = 0; i < l; i++)
        c[i] = t[i];
}

/**
 * Interleave the data codewords.
 *
 * @param l data codewords length.
 * @param d data codewords.
 * @param b reed-solomon block information.
 */
static void interleave_data_codewords(int l, uint8_t d[], rs_block_info_t b) {
    const uint8_t *p[b.num_blocks1 + b.num_blocks2];

    for (int i = 0; i < b.num_blocks1; i++)
        p[i] = &d[b.num_data_codewords1 * i];

    for (int i = 0; i < b.num_blocks2; i++)
        p[b.num_blocks1 + i] = &d[b.num_data_codewords1 * b.num_blocks1 +
                                  b.num_data_codewords2 * i];

    interleave_codewords(l, d, b.num_blocks1, b.num_blocks2,
                         b.num_data_codewords1, p);
}

/**
 * Interleave the error correction codewords.
 *
 * @param l error correction codewords length.
 * @param e error correction codewords.
 * @param b reed-solomon block information.
 */
static void interleave_ec_codewords(int l, uint8_t e[], rs_block_info_t b) {
    const uint8_t *p[b.num_blocks1 + b.num_blocks2];

    for (int i = 0; i < b.num_blocks1 + b.num_blocks2; i++)
        p[i] = &e[b.num_ec_codewords * i];

    interleave_codewords(l, e, b.num_blocks1 + b.num_blocks2, 0,
                         b.num_ec_codewords, p);
}

/**
 * Returns reed-solomon block information.
 *
 * @param v version.
 * @param e error correction level.
 * @return reed-solomon block information.
 */
rs_block_info_t rs_block_information(int v, error_correction_level_t e) {
    const int8_t *b = rs_block_table[v][e];

    return (rs_block_info_t){
        (num_codewords[v] - (b[0] * b[1] + b[2] * (b[1] + 1))) / (b[0] + b[2]),
        b[0], b[1], b[2], b[2] == 0 ? 0 : b[1] + 1};
}

/**
 * Build the final message.
 *
 * @param f final message.
 * @param d data codewords.
 * @param e error correction codewords.
 * @param b reed-solomon block information.
 */
void build_final_message(uint8_t f[], uint8_t d[], uint8_t e[],
                         rs_block_info_t b) {
    int l = b.num_data_codewords1 * b.num_blocks1 +
            b.num_data_codewords2 * b.num_blocks2;
    int n = b.num_ec_codewords * (b.num_blocks1 + b.num_blocks2);

    interleave_data_codewords(l, d, b);
    interleave_ec_codewords(n, e, b);

    for (int i = 0; i < l; i++)
        f[i] = d[i];

    for (int i = 0; i < n; i++)
        f[l + i] = e[i];

    // remainder bits
    f[l + n] = 0;
}
