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
 * @file main.c
 * @brief main implementation
 */
#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "gf256.h"
#include "image.h"
#include "mask.h"
#include "message.h"
#include "module.h"

#define MAX_DATA_LENGTH 7089

static const char *ec_levels = "LMQH";

int main(int argc, char const *argv[]) {
    error_correction_level_t ec_level = ERROR_CORRECTION_LEVEL_L;

    char option = 0;
    char *level;

    for (int i = 1; i < argc; i++) {
        char const *argp = argv[i];

        if (argp[0] == '-') {
            if (option != 0) {
                fprintf(stderr, "illegal option argument: %s\n", argp);
                return 0;
            }

            switch (argp[1]) {
            case 'e':
                option = argp[1];
                break;

            default:
                fprintf(stderr, "illegal option: %s\n", argp);
                return 0;
            }

            if (argp[2] == '\0')
                continue;
            else
                argp += 2;
        }

        switch (option) {
        case 'e':
            level = strchr(ec_levels, (unsigned char)argp[0]);

            if (level == NULL || argp[1] != '\0') {
                fprintf(stderr, "illegal option argument: %s\n", argp);
                return 0;
            }

            ec_level = level - ec_levels;
            break;

        default:
            fprintf(stderr, "illegal option: %s\n", argp);
            return 0;
        }

        option = 0;
    }

    if (option != 0) {
        fprintf(stderr, "illegal option: %s\n", argv[argc - 1]);
        return 0;
    }

    uint8_t data[MAX_DATA_LENGTH + 1];
    int data_length = fread(data, sizeof(uint8_t), MAX_DATA_LENGTH + 1, stdin);

    if (data_length <= 0) {
        fprintf(stderr, "file read error\n");
        return 0;
    }

    encoding_mode_t encoding_mode = min_encoding_mode(data_length, data);
    int version = min_version(data_length, ec_level, encoding_mode);

    if (version < 0) {
        fprintf(stderr, "data is too long\n");
        return 0;
    }

    int data_codewords_length = num_data_codewords(version, ec_level);

    uint8_t data_codewords[data_codewords_length];

    encode(data_codewords_length, data_codewords, data_length, data, version,
           encoding_mode);

    rs_block_info_t rs_block_info = rs_block_information(version, ec_level);

    int genpoly_length = rs_block_info.num_ec_codewords;
    uint8_t genpoly[genpoly_length];

    gf256_genpoly(genpoly_length, genpoly);

    int ec_codewords_length =
        rs_block_info.num_ec_codewords *
        (rs_block_info.num_blocks1 + rs_block_info.num_blocks2);

    uint8_t ec_codewords[ec_codewords_length];

    int d_index = 0;
    int e_index = 0;

    for (int i = 0; i < rs_block_info.num_blocks1; i++) {
        gf256_divpoly(&ec_codewords[e_index], rs_block_info.num_data_codewords1,
                      &data_codewords[d_index], genpoly_length, genpoly);

        d_index += rs_block_info.num_data_codewords1;
        e_index += rs_block_info.num_ec_codewords;
    }

    for (int i = 0; i < rs_block_info.num_blocks2; i++) {
        gf256_divpoly(&ec_codewords[e_index], rs_block_info.num_data_codewords2,
                      &data_codewords[d_index], genpoly_length, genpoly);

        d_index += rs_block_info.num_data_codewords2;
        e_index += rs_block_info.num_ec_codewords;
    }

    int final_message_length = data_codewords_length + ec_codewords_length + 1;
    uint8_t final_message[final_message_length];

    build_final_message(final_message, data_codewords, ec_codewords,
                        rs_block_info);

    int qr_length = matrix_length(version);
    module_type_t matrix[qr_length][qr_length];
    bool mask_flags[qr_length][qr_length];

    place_modules(qr_length, matrix, mask_flags, final_message, version);
    mask_modules_auto(qr_length, matrix, matrix, mask_flags, ec_level);

    // Add the quiet zone
    int qr_image_length = qr_length + 8;
    module_type_t qr_image[qr_image_length][qr_image_length];

    for (int i = 0; i < qr_image_length; i++)
        for (int j = 0; j < qr_image_length; j++)
            qr_image[i][j] = MODULE_TYPE_LIGHT;

    for (int i = 0; i < qr_length; i++)
        for (int j = 0; j < qr_length; j++)
            qr_image[i + 4][j + 4] = matrix[i][j];

    write_bmp(qr_image_length, qr_image, stdout);

    return 0;
}
