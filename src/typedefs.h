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
 * @file typedefs.h
 * @brief typedefs header
 */
#ifndef TYPEDEFS_H
#define TYPEDEFS_H

typedef enum {
    ERROR_CORRECTION_LEVEL_L,
    ERROR_CORRECTION_LEVEL_M,
    ERROR_CORRECTION_LEVEL_Q,
    ERROR_CORRECTION_LEVEL_H
} error_correction_level_t;

typedef enum {
    ENCODING_MODE_NUMERIC,
    ENCODING_MODE_ALPHANUMERIC,
    ENCODING_MODE_BYTE,
    ENCODING_MODE_KANJI
} encoding_mode_t;

typedef enum {
    MODULE_TYPE_NONE = -1,
    MODULE_TYPE_LIGHT = 0,
    MODULE_TYPE_DARK = 1,
    MODULE_TYPE_RESERVED = 2
} module_type_t;

#endif /* TYPEDEFS_H */
