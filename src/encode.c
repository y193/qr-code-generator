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
 * @file encode.c
 * @brief encode implementation
 */
#include <stdbool.h>
#include "encode.h"

#define MODE_INDICATOR_LEN (4)

#define append_bits(l, b, n, v)                                                \
    do {                                                                       \
        b <<= n;                                                               \
        b |= v;                                                                \
        l += n;                                                                \
    } while (0)

static const int8_t chrcnt_indicator_lens[][4] = {
    {10, 9, 8, 8}, {12, 11, 16, 10}, {14, 13, 16, 12}};

static const int8_t alphanumerics[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 36, -1, -1, -1, 37, 38,
    -1, -1, -1, -1, 39, 40, -1, 41, 42, 43, 0,  1,  2,  3,  4,  5,  6,  7,  8,
    9,  44, -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

static const int16_t num_dat_codewords[][4] = {
    {19, 16, 13, 9},          {34, 28, 22, 16},
    {55, 44, 34, 26},         {80, 64, 48, 36},
    {108, 86, 62, 46},        {136, 108, 76, 60},
    {156, 124, 88, 66},       {194, 154, 110, 86},
    {232, 182, 132, 100},     {274, 216, 154, 122},
    {324, 254, 180, 140},     {370, 290, 206, 158},
    {428, 334, 244, 180},     {461, 365, 261, 197},
    {523, 415, 295, 223},     {589, 453, 325, 253},
    {647, 507, 367, 283},     {721, 563, 397, 313},
    {795, 627, 445, 341},     {861, 669, 485, 385},
    {932, 714, 512, 406},     {1006, 782, 568, 442},
    {1094, 860, 614, 464},    {1174, 914, 664, 514},
    {1276, 1000, 718, 538},   {1370, 1062, 754, 596},
    {1468, 1128, 808, 628},   {1531, 1193, 871, 661},
    {1631, 1267, 911, 701},   {1735, 1373, 985, 745},
    {1843, 1455, 1033, 793},  {1955, 1541, 1115, 845},
    {2071, 1631, 1171, 901},  {2191, 1725, 1231, 961},
    {2306, 1812, 1286, 986},  {2434, 1914, 1354, 1054},
    {2566, 1992, 1426, 1096}, {2702, 2102, 1502, 1142},
    {2812, 2216, 1582, 1222}, {2956, 2334, 1666, 1276}};

/**
 * Number of character count indicator bits by version and encoding mode.
 *
 * @param v version.
 * @param m encoding mode.
 * @return number of character count indicator bits.
 */
static int chrcnt_indicator_len(int v, encoding_mode_t m) {
    return chrcnt_indicator_lens[(v >= 9) + (v >= 26)][m];
}

/**
 * Calculate character capacity by number of data bits (excluding mode indicator
 * and character count indicator) and encoding mode.
 *
 * @param n number of data bits.
 * @param m encoding mode.
 * @return character capacity.
 */
static int calc_chr_capacity(int n, encoding_mode_t m) {
    switch (m) {
    case ENCODING_MODE_NUMERIC:
        return n / 10 * 3 + n % 10 / 7 * 2 + n % 10 % 7 / 4;

    case ENCODING_MODE_ALPHANUMERIC:
        return n / 11 * 2 + n % 11 / 6;

    case ENCODING_MODE_BYTE:
        return n / 8;

    case ENCODING_MODE_KANJI:
        return n / 13;

    default:
        return -1;
    }
}

/**
 * Returns true if the character is a double-byte character in the Shift_JIS
 * character set.
 *
 * @param f first byte.
 * @param s second byte.
 * @return true if the character is a double-byte character in the Shift_JIS
 * character set and false otherwise.
 */
static bool is_sjis_kanji(uint8_t f, uint8_t s) {
    return ((0x81 <= f && f <= 0x9F) || (0xE0 <= f && f <= 0xEB)) &&
           ((0x40 <= s && s < 0x7F) || (0x7F < s && s <= 0xFC)) &&
           (f << 8 | s) <= 0xEBBF;
}

/**
 * Add terminator, padding bits and pad codewords.
 *
 * @param n data codewords length.
 * @param d data codewords.
 * @param i index of data codewords.
 * @param k buffer length.
 * @param b buffer.
 */
static void terminate_data_codewords(int n, uint8_t d[], int i, int k,
                                     uint32_t b) {
    int r = (n - i) * 8 - k;

    // terminator
    int t = r < 4 ? r : 4;
    b <<= t;
    k += t;
    r -= t;

    // add padding bits
    r %= 8;
    b <<= r;
    k += r;

    while (k > 0) {
        k -= 8;
        d[i++] = b >> k;
    }

    // add pad codewords
    uint8_t p = 0xEC;

    while (i < n) {
        d[i++] = p;
        p ^= 0xFD;
    }
}

/**
 * Encode in numeric mode.
 *
 * @param n data codewords length.
 * @param d data codewords.
 * @param l input string length.
 * @param s input string.
 * @param c character count indicator length.
 */
static void encode_numeric_mode(int n, uint8_t d[], int l, const uint8_t s[],
                                int c) {
    // mode indicator
    uint32_t b = 0x1;
    int k = MODE_INDICATOR_LEN;

    // character count indicator
    append_bits(k, b, c, l);

    // encoded data
    int j = 0;

    for (int i = 2; i < l; i += 3) {
        append_bits(
            k, b, 10,
            ((s[i - 2] - '0') * 100 + (s[i - 1] - '0') * 10 + (s[i] - '0')));

        do {
            k -= 8;
            d[j++] = b >> k;
        } while (k >= 8);
    }

    if (l % 3 == 2)
        append_bits(k, b, 7, ((s[l - 2] - '0') * 10 + (s[l - 1] - '0')));

    else if (l % 3 == 1)
        append_bits(k, b, 4, (s[l - 1] - '0'));

    // terminator, padding bits and pad codewords
    terminate_data_codewords(n, d, j, k, b);
}

/**
 * Encode in alphanumeric mode.
 *
 * @param n data codewords length.
 * @param d data codewords.
 * @param l input string length.
 * @param s input string.
 * @param c character count indicator length.
 */
static void encode_alphanumeric_mode(int n, uint8_t d[], int l,
                                     const uint8_t s[], int c) {
    // mode indicator
    uint32_t b = 0x2;
    int k = MODE_INDICATOR_LEN;

    // character count indicator
    append_bits(k, b, c, l);

    // encoded data
    int j = 0;

    for (int i = 1; i < l; i += 2) {
        append_bits(k, b, 11,
                    (alphanumerics[s[i - 1]] * 45 + alphanumerics[s[i]]));

        do {
            k -= 8;
            d[j++] = b >> k;
        } while (k >= 8);
    }

    if (l % 2 == 1)
        append_bits(k, b, 6, alphanumerics[s[l - 1]]);

    // terminator, padding bits and pad codewords
    terminate_data_codewords(n, d, j, k, b);
}

/**
 * Encode in byte mode.
 *
 * @param n data codewords length.
 * @param d data codewords.
 * @param l input string length.
 * @param s input string.
 * @param c character count indicator length.
 */
static void encode_byte_mode(int n, uint8_t d[], int l, const uint8_t s[],
                             int c) {
    // mode indicator
    uint32_t b = 0x4;

    // character count indicator
    b <<= c;
    b |= l;

    if (c == 8) {
        d[0] = b >> 4;
        d[1] = b << 4;

    } else {
        d[0] = b >> 12;
        d[1] = b >> 4;
        d[2] = b << 4;
    }

    // encoded data
    for (int i = 0; i < l; i++) {
        d[c / 8 + i] |= s[i] >> 4;
        d[c / 8 + i + 1] = s[i] << 4;
    }

    // terminator, padding bits and pad codewords
    terminate_data_codewords(n, d, c / 8 + l, 4, s[l - 1]);
}

/**
 * Encode in kanji mode.
 *
 * @param n data codewords length.
 * @param d data codewords.
 * @param l input string length.
 * @param s input string.
 * @param c character count indicator length.
 */
static void encode_kanji_mode(int n, uint8_t d[], int l, const uint8_t s[],
                              int c) {
    // mode indicator
    uint32_t b = 0x8;
    int k = MODE_INDICATOR_LEN;

    // character count indicator
    append_bits(k, b, c, l);

    // encoded data
    int j = 0;

    for (int i = 1; i < l; i += 2) {
        append_bits(
            k, b, 13,
            ((s[i - 1] - ((s[i - 1] & 0xC0) | 0x01) - (s[i] < 0x40)) * 0xC0 +
             (s[i] - 0x40)));

        do {
            k -= 8;
            d[j++] = b >> k;
        } while (k >= 8);
    }

    // terminator, padding bits and pad codewords
    terminate_data_codewords(n, d, j, k, b);
}

/**
 * The most efficient mode for data.
 *
 * @param l input string length.
 * @param s input string.
 * @return encoding mode.
 */
encoding_mode_t min_encoding_mode(int l, const uint8_t s[]) {
    int i = 0;

    while (i < l && '0' <= s[i] && s[i] <= '9')
        i++;

    if (i == l)
        return ENCODING_MODE_NUMERIC;

    while (i < l && s[i] < 0x80 && alphanumerics[s[i]] >= 0)
        i++;

    if (i == l)
        return ENCODING_MODE_ALPHANUMERIC;

    if (i != 0 || l % 2 != 0)
        return ENCODING_MODE_BYTE;

    for (int i = 0; i < l; i += 2)
        if (!is_sjis_kanji(s[i], s[i + 1]))
            return ENCODING_MODE_BYTE;

    return ENCODING_MODE_KANJI;
}

/**
 * The smallest version for data.
 *
 * @param l input string length.
 * @param e error correction level.
 * @param m encoding mode.
 * @return version.
 */
int min_version(int l, error_correction_level_t e, encoding_mode_t m) {
    for (int v = 0; v < 40; v++) {
        int d = num_dat_codewords[v][e] * 8;
        int i = MODE_INDICATOR_LEN + chrcnt_indicator_len(v, m);

        if (l <= calc_chr_capacity(d - i, m))
            return v;
    }

    return -1;
}

/**
 * Total number of data codewords by version, error correction level.
 *
 * @param v version.
 * @param e error correction level.
 * @return total number of data codewords.
 */
int num_data_codewords(int v, error_correction_level_t e) {
    return num_dat_codewords[v][e];
}

/**
 * Encode using the selected mode.
 *
 * @param n data codewords length.
 * @param d data codewords.
 * @param l input string length.
 * @param s input string.
 * @param v version.
 * @param m encoding mode.
 */
void encode(int n, uint8_t d[], int l, const uint8_t s[], int v,
            encoding_mode_t m) {
    for (int i = 0; i < n; i++)
        d[i] = 0;

    int c = chrcnt_indicator_len(v, m);

    switch (m) {
    case ENCODING_MODE_NUMERIC:
        encode_numeric_mode(n, d, l, s, c);
        break;

    case ENCODING_MODE_ALPHANUMERIC:
        encode_alphanumeric_mode(n, d, l, s, c);
        break;

    case ENCODING_MODE_BYTE:
        encode_byte_mode(n, d, l, s, c);
        break;

    case ENCODING_MODE_KANJI:
        encode_kanji_mode(n, d, l, s, c);
        break;
    }
}
