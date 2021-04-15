#include <assert.h>
#include "encode.h"

static void test_min_encoding_mode(void) {
    assert(min_encoding_mode(0, (uint8_t *)"") == ENCODING_MODE_NUMERIC);
    assert(min_encoding_mode(1, (uint8_t *)"0") == ENCODING_MODE_NUMERIC);
    assert(min_encoding_mode(10, (uint8_t *)"0123456789") == ENCODING_MODE_NUMERIC);

    assert(min_encoding_mode(1, (uint8_t *)"A") == ENCODING_MODE_ALPHANUMERIC);
    assert(min_encoding_mode(3, (uint8_t *)"0.1") == ENCODING_MODE_ALPHANUMERIC);
    assert(min_encoding_mode(45, (uint8_t *)"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:") == ENCODING_MODE_ALPHANUMERIC);

    assert(min_encoding_mode(1, (uint8_t *)"a") == ENCODING_MODE_BYTE);
    assert(min_encoding_mode(1, (uint8_t *)"#") == ENCODING_MODE_BYTE);
    assert(min_encoding_mode(5, (uint8_t *)"1,000") == ENCODING_MODE_BYTE);

    assert(min_encoding_mode(2, (uint8_t[]){0x82, 0xA0}) == ENCODING_MODE_KANJI);
    assert(min_encoding_mode(4, (uint8_t[]){0x93, 0xFA, 0x96, 0x7B}) == ENCODING_MODE_KANJI);

    assert(min_encoding_mode(4, (uint8_t[]){' ', ' ', 0x82, 0xA0}) == ENCODING_MODE_BYTE);
    assert(min_encoding_mode(3, (uint8_t[]){0x82, 0xA0, ' '}) == ENCODING_MODE_BYTE);

    assert(min_encoding_mode(2, (uint8_t[]){0x81, 0x3F}) == ENCODING_MODE_BYTE);
    assert(min_encoding_mode(2, (uint8_t[]){0x81, 0x40}) == ENCODING_MODE_KANJI);
    assert(min_encoding_mode(2, (uint8_t[]){0x81, 0x7E}) == ENCODING_MODE_KANJI);
    assert(min_encoding_mode(2, (uint8_t[]){0x81, 0x7F}) == ENCODING_MODE_BYTE);
    assert(min_encoding_mode(2, (uint8_t[]){0x81, 0x80}) == ENCODING_MODE_KANJI);
    assert(min_encoding_mode(2, (uint8_t[]){0x81, 0xFC}) == ENCODING_MODE_KANJI);
    assert(min_encoding_mode(2, (uint8_t[]){0x81, 0xFD}) == ENCODING_MODE_BYTE);
    assert(min_encoding_mode(2, (uint8_t[]){0x9F, 0xFC}) == ENCODING_MODE_KANJI);
    assert(min_encoding_mode(2, (uint8_t[]){0xA0, 0x40}) == ENCODING_MODE_BYTE);
    assert(min_encoding_mode(2, (uint8_t[]){0xDF, 0xFC}) == ENCODING_MODE_BYTE);
    assert(min_encoding_mode(2, (uint8_t[]){0xE0, 0x40}) == ENCODING_MODE_KANJI);
    assert(min_encoding_mode(2, (uint8_t[]){0xEB, 0xBF}) == ENCODING_MODE_KANJI);
    assert(min_encoding_mode(2, (uint8_t[]){0xEB, 0xC0}) == ENCODING_MODE_BYTE);
}

int main(int argc, char const *argv[]) {
    test_min_encoding_mode();

    return 0;
}
