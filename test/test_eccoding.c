#include <assert.h>
#include "gf256.h"

static void test_gf256_genpoly(void) {
    const uint8_t expected_g7[] = {21, 102, 238, 149, 146, 229, 87};
    const uint8_t expected_g10[] = {45, 32, 94, 64, 70, 118, 61, 46, 67, 251};
    const uint8_t expected_g30[] = {180, 192, 40,  238, 216, 251, 37,  156,
                                    130, 224, 193, 226, 173, 42,  125, 222,
                                    96,  239, 86,  110, 48,  50,  182, 179,
                                    31,  216, 152, 145, 173, 41};
    const uint8_t expected_g68[] = {
        238, 163, 8,   5,   3,   127, 184, 101, 27,  235, 238, 43, 198, 175,
        215, 82,  32,  54,  2,   118, 225, 166, 241, 137, 125, 41, 177, 52,
        231, 95,  97,  199, 52,  227, 89,  160, 173, 253, 84,  15, 84,  93,
        151, 203, 220, 165, 202, 60,  52,  133, 205, 190, 101, 84, 150, 43,
        254, 32,  160, 90,  70,  77,  93,  224, 33,  223, 159, 247};
    uint8_t g7[7];
    uint8_t g10[10];
    uint8_t g30[30];
    uint8_t g68[68];

    gf256_genpoly(7, g7);
    gf256_genpoly(10, g10);
    gf256_genpoly(30, g30);
    gf256_genpoly(68, g68);

    for (int i = 0; i < 7; i++)
        assert(g7[i] == expected_g7[i]);

    for (int i = 0; i < 10; i++)
        assert(g10[i] == expected_g10[i]);

    for (int i = 0; i < 30; i++)
        assert(g30[i] == expected_g30[i]);

    for (int i = 0; i < 68; i++)
        assert(g68[i] == expected_g68[i]);
}

static void test_gf256_divpoly_40L(void) {
    const uint8_t expected[] = {
        189, 137, 252, 139, 254, 105, 3,  191, 148, 6,  155, 201, 191, 108, 82,
        125, 222, 189, 22,  236, 248, 95, 241, 244, 49, 33,  139, 154, 162, 89};
    const uint8_t m_g1b1[] = {
        22,  236, 83,  162, 125, 9,   89,  189, 53,  15,  78,  66,  21,  37,
        31,  182, 72,  26,  59,  51,  171, 234, 95,  233, 24,  97,  123, 156,
        179, 155, 13,  160, 39,  73,  179, 131, 215, 178, 55,  138, 22,  52,
        236, 44,  120, 214, 202, 40,  181, 32,  230, 166, 44,  86,  1,   204,
        238, 195, 163, 124, 172, 133, 250, 204, 179, 80,  27,  238, 144, 113,
        154, 67,  140, 29,  80,  110, 138, 252, 71,  57,  38,  223, 222, 208,
        152, 196, 110, 183, 17,  134, 54,  148, 85,  137, 147, 79,  141, 51,
        117, 234, 166, 79,  32,  241, 224, 190, 228, 163, 77,  98,  180, 86,
        130, 45,  116, 45,  108, 41};
    const uint8_t g[] = {180, 192, 40,  238, 216, 251, 37,  156, 130, 224,
                         193, 226, 173, 42,  125, 222, 96,  239, 86,  110,
                         48,  50,  182, 179, 31,  216, 152, 145, 173, 41};
    uint8_t r_g1b1[30];

    gf256_divpoly(r_g1b1, 118, m_g1b1, 30, g);

    for (int i = 0; i < 30; i++)
        assert(r_g1b1[i] == expected[i]);
}

static void test_gf256_divpoly_discard0_first(void) {
    const uint8_t expected[] = {246, 245, 185, 37, 184, 15,  88, 175,
                                173, 61,  128, 31, 198, 229, 88, 155,
                                189, 94,  140, 76, 221, 160};

    // block 2 of "00000000000000003XN*000000000000000" encoded in version
    // 3-H
    const uint8_t m_g1b2[] = {168, 134, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    const uint8_t g[] = {231, 165, 105, 160, 134, 219, 80, 98,  172, 8,   74,
                         200, 53,  221, 109, 14,  230, 93, 242, 247, 171, 210};
    uint8_t r_g1b2[22];

    gf256_divpoly(r_g1b2, 13, m_g1b2, 22, g);

    for (int i = 0; i < 22; i++)
        assert(r_g1b2[i] == expected[i]);
}

static void test_gf256_divpoly_discard0_last(void) {
    const uint8_t expected[] = {0,   89,  130, 86,  175, 135, 227, 55,
                                134, 216, 30,  221, 158, 25,  137, 173,
                                93,  197, 114, 223, 235, 110};

    // block 2 of "00000000000000003XV2/9MA49*[SP]WU.0WA[SP]" encoded in version
    // 3-H
    const uint8_t m_g1b2[] = {168, 174, 190, 97, 244, 11, 221,
                              255, 111, 187, 21, 170, 144};

    const uint8_t g[] = {231, 165, 105, 160, 134, 219, 80, 98,  172, 8,   74,
                         200, 53,  221, 109, 14,  230, 93, 242, 247, 171, 210};
    uint8_t r_g1b2[22];

    gf256_divpoly(r_g1b2, 13, m_g1b2, 22, g);

    for (int i = 0; i < 22; i++)
        assert(r_g1b2[i] == expected[i]);
}

static void test_gf256_divpoly_divide0(void) {
    const uint8_t expected[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    // block 2 of "A0000000000000000000000000000000000" encoded in version
    // 3-H
    const uint8_t m_g1b2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    const uint8_t g[] = {231, 165, 105, 160, 134, 219, 80, 98,  172, 8,   74,
                         200, 53,  221, 109, 14,  230, 93, 242, 247, 171, 210};
    uint8_t r_g1b2[22];

    gf256_divpoly(r_g1b2, 13, m_g1b2, 22, g);

    for (int i = 0; i < 22; i++)
        assert(r_g1b2[i] == expected[i]);
}

int main(int argc, char const *argv[]) {
    test_gf256_genpoly();

    test_gf256_divpoly_40L();
    test_gf256_divpoly_discard0_first();
    test_gf256_divpoly_discard0_last();
    test_gf256_divpoly_divide0();

    return 0;
}
