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
 * @file image.c
 * @brief image implementation
 */
#include <stdint.h>
#include "image.h"

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;

typedef struct tagBITMAPFILEHEADER {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER, *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagRGBQUAD {
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
} RGBQUAD;

static void write_word_le(uint8_t *d, WORD s) {
    d[0] = s & 0xFF;
    d[1] = (s >> 8) & 0xFF;
}

static void write_dword_le(uint8_t *d, DWORD s) {
    d[0] = s & 0xFF;
    d[1] = (s >> 8) & 0xFF;
    d[2] = (s >> 16) & 0xFF;
    d[3] = (s >> 24) & 0xFF;
}

static void write_long_le(uint8_t *d, LONG s) {
    d[0] = s & 0xFF;
    d[1] = (s >> 8) & 0xFF;
    d[2] = (s >> 16) & 0xFF;
    d[3] = (s >> 24) & 0xFF;
}

void write_bmp(int n, module_type_t m[][n], FILE *f) {
    int s = ((n + 31) & ~31) >> 3;
    uint32_t l = 62 + n * s;
    uint8_t d[l];

    BITMAPFILEHEADER bf = {0x4D42, l, 0, 0, 62};

    write_word_le(&d[0], bf.bfType);
    write_dword_le(&d[2], bf.bfSize);
    write_word_le(&d[6], bf.bfReserved1);
    write_word_le(&d[8], bf.bfReserved2);
    write_dword_le(&d[10], bf.bfOffBits);

    BITMAPINFOHEADER bi = {40, n, n, 1, 1, 0, 0, 0, 0, 0, 0};

    write_dword_le(&d[14], bi.biSize);
    write_long_le(&d[18], bi.biWidth);
    write_long_le(&d[22], bi.biHeight);
    write_word_le(&d[26], bi.biPlanes);
    write_word_le(&d[28], bi.biBitCount);
    write_dword_le(&d[30], bi.biCompression);
    write_dword_le(&d[34], bi.biSizeImage);
    write_long_le(&d[38], bi.biXPelsPerMeter);
    write_long_le(&d[42], bi.biYPelsPerMeter);
    write_dword_le(&d[46], bi.biClrUsed);
    write_dword_le(&d[50], bi.biClrImportant);

    // color table
    RGBQUAD rgbWhite = {0xFF, 0xFF, 0xFF, 0};
    RGBQUAD rgbBlack = {0, 0, 0, 0};

    d[54] = rgbWhite.rgbBlue;
    d[55] = rgbWhite.rgbGreen;
    d[56] = rgbWhite.rgbRed;
    d[57] = rgbWhite.rgbReserved;

    d[58] = rgbBlack.rgbBlue;
    d[59] = rgbBlack.rgbGreen;
    d[60] = rgbBlack.rgbRed;
    d[61] = rgbBlack.rgbReserved;

    // pixel data
    int k = 62;

    for (int i = n - 1; i >= 0; i--) {
        uint8_t b = 0;

        for (int j = 0; j < n; j++) {
            b <<= 1;
            b |= m[i][j];

            if (j % 8 == 7) {
                d[k++] = b;
                b = 0;
            }
        }

        d[k++] = b << (8 - n % 8);

        for (int j = n / 8 + 1; j < s; j++)
            d[k++] = 0;
    }

    fwrite(d, sizeof(uint8_t), l, f);
}
