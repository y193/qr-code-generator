#include <assert.h>
#include "mask.h"

static void test_eval_penalty_1to3(void) {

    // "HELLO WORLD" encoded in version 1-Q
    const module_type_t matrix[][21] = {
        {1, 1, 1, 1, 1, 1, 1, 0, 2, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 1, 0, 2, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
        {1, 0, 1, 1, 1, 0, 1, 0, 2, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1},
        {1, 0, 1, 1, 1, 0, 1, 0, 2, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1},
        {1, 0, 1, 1, 1, 0, 1, 0, 2, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1},
        {1, 0, 0, 0, 0, 0, 1, 0, 2, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {2, 2, 2, 2, 2, 2, 1, 2, 2, 0, 1, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2},
        {0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1},
        {1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1},
        {0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 0, 2, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 1, 0, 2, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0},
        {1, 0, 1, 1, 1, 0, 1, 0, 2, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0},
        {1, 0, 1, 1, 1, 0, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
        {1, 0, 1, 1, 1, 0, 1, 0, 2, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0},
        {1, 0, 0, 0, 0, 0, 1, 0, 2, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 0, 2, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 0}};

    // modules that mask is applied
    const bool flags[][21] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

    module_type_t masked[21][21];

    mask_modules(21, masked, matrix, flags, ERROR_CORRECTION_LEVEL_Q, 0);
    assert(eval_penalty(21, masked) == 347);

    mask_modules(21, masked, matrix, flags, ERROR_CORRECTION_LEVEL_Q, 1);
    assert(eval_penalty(21, masked) == 470);

    mask_modules(21, masked, matrix, flags, ERROR_CORRECTION_LEVEL_Q, 2);
    assert(eval_penalty(21, masked) == 506);

    mask_modules(21, masked, matrix, flags, ERROR_CORRECTION_LEVEL_Q, 3);
    assert(eval_penalty(21, masked) == 441);

    mask_modules(21, masked, matrix, flags, ERROR_CORRECTION_LEVEL_Q, 4);
    assert(eval_penalty(21, masked) == 539);

    mask_modules(21, masked, matrix, flags, ERROR_CORRECTION_LEVEL_Q, 5);
    assert(eval_penalty(21, masked) == 516);

    mask_modules(21, masked, matrix, flags, ERROR_CORRECTION_LEVEL_Q, 6);
    assert(eval_penalty(21, masked) == 314);

    mask_modules(21, masked, matrix, flags, ERROR_CORRECTION_LEVEL_Q, 7);
    assert(eval_penalty(21, masked) == 558);

    mask_modules_auto(21, masked, matrix, flags, ERROR_CORRECTION_LEVEL_Q);
    assert(eval_penalty(21, masked) == 314);
}

static void test_eval_penalty_4(void) {

    // the percentage of dark modules is 25%
    const module_type_t matrix_25[][4] = {
        {0, 0, 0, 0},
        {0, 1, 0, 1},
        {0, 0, 0, 0},
        {0, 1, 0, 1}};

    // the percentage of dark modules is 50%
    const module_type_t matrix_50[][4] = {
        {1, 0, 1, 0},
        {0, 1, 0, 1},
        {1, 0, 1, 0},
        {0, 1, 0, 1}};

    // the percentage of dark modules is 62.5%
    const module_type_t matrix_62[][4] = {
        {1, 1, 1, 0},
        {0, 1, 0, 1},
        {1, 0, 1, 0},
        {0, 1, 1, 1}};

    // the percentage of dark modules is 100% + (penalty2 * 9)
    const module_type_t matrix_100[][4] = {
        {1, 1, 1, 1},
        {1, 1, 1, 1},
        {1, 1, 1, 1},
        {1, 1, 1, 1}};

    assert(eval_penalty(4, matrix_25) == 50);
    assert(eval_penalty(4, matrix_50) == 0);
    assert(eval_penalty(4, matrix_62) == 20);
    assert(eval_penalty(4, matrix_100) == 127);
}

int main(int argc, char const *argv[]) {
    test_eval_penalty_1to3();
    test_eval_penalty_4();

    return 0;
}
