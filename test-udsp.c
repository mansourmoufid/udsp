/* Copyright 2013-2015, Mansour Moufid <mansourmoufid@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#undef NDEBUG

#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fltop.h"
#include "nclock.h"
#include "udsp.h"

#define COMPLEX_EQUALS(x, y) \
        (flt_eq(x.real, y.real) && flt_eq(x.imag, y.imag))

#define TEST_INPUT_LENGTH 10
#define REL_ERR_MAX 5e-2f

static float test_input[TEST_INPUT_LENGTH] = {
     1.00000000f,
     2.00000000f,
     3.00000000f,
     4.00000000f,
     5.00000000f,
     6.00000000f,
     7.00000000f,
     8.00000000f,
     9.00000000f,
    10.00000000f,
};

static udsp_complex_t fft_output[TEST_INPUT_LENGTH];
static float conv_output[2 * TEST_INPUT_LENGTH];
static float xcov_output[2 * TEST_INPUT_LENGTH];
static float xcor_output[2 * TEST_INPUT_LENGTH];
static float pow_output[TEST_INPUT_LENGTH];

#define N_FFT_TEST_CASES    4
#define N_CONV_TEST_CASES   4
#define N_XCOV_TEST_CASES   4
#define N_XCOR_TEST_CASES   4
#define N_POW_TEST_CASES    4

static udsp_complex_t fft_test_cases[][TEST_INPUT_LENGTH] = {
    {{  1.00000000f,   0.00000000f}},
    {{  3.00000000f,   0.00000000f},
     { -1.00000000f,   0.00000000f}},
    {{  6.00000000f,   0.00000000f},
     { -1.50000000f,   0.86602540f},
     { -1.50000000f,  -0.86602540f}},
    {{ 10.00000000f,   0.00000000f},
     { -2.00000000f,   2.00000000f},
     { -2.00000000f,   0.00000000f},
     { -2.00000000f,  -2.00000000f}},
};

static float conv_test_cases[][2 * TEST_INPUT_LENGTH] = {
    {  1.00000000f,
       2.00000000f,
       3.00000000f,
       4.00000000f,
       5.00000000f,
       6.00000000f,
       7.00000000f,
       8.00000000f,
       9.00000000f,
      10.00000000f,},
    {  1.00000000f,
       4.00000000f,
       7.00000000f,
      10.00000000f,
      13.00000000f,
      16.00000000f,
      19.00000000f,
      22.00000000f,
      25.00000000f,
      28.00000000f,
      20.00000000f,},
    {  1.00000000f,
       4.00000000f,
      10.00000000f,
      16.00000000f,
      22.00000000f,
      28.00000000f,
      34.00000000f,
      40.00000000f,
      46.00000000f,
      52.00000000f,
      47.00000000f,
      30.00000000f,},
    {  1.00000000f,
       4.00000000f,
      10.00000000f,
      20.00000000f,
      30.00000000f,
      40.00000000f,
      50.00000000f,
      60.00000000f,
      70.00000000f,
      80.00000000f,
      79.00000000f,
      66.00000000f,
      40.00000000f,},
};

static float xcov_test_cases[][2 * TEST_INPUT_LENGTH] = {
    {  0.10000000f,
       0.20000000f,
       0.30000000f,
       0.40000000f,
       0.50000000f,
       0.60000000f,
       0.70000000f,
       0.80000000f,
       0.90000000f,
       1.00000000f,},
    {  0.20000000f,
       0.50000000f,
       0.80000000f,
       1.10000000f,
       1.40000000f,
       1.70000000f,
       2.00000000f,
       2.30000000f,
       2.60000000f,
       2.90000000f,
       1.00000000f,},
    {  0.30000000f,
       0.80000000f,
       1.40000000f,
       2.00000000f,
       2.60000000f,
       3.20000000f,
       3.80000000f,
       4.40000000f,
       5.00000000f,
       5.60000000f,
       2.90000000f,
       1.00000000f,},
    {  0.40000000f,
       1.10000000f,
       2.00000000f,
       3.00000000f,
       4.00000000f,
       5.00000000f,
       6.00000000f,
       7.00000000f,
       8.00000000f,
       9.00000000f,
       5.60000000f,
       2.90000000f,
       1.00000000f,},
};

static float xcor_test_cases[][2 * TEST_INPUT_LENGTH] = {
    {  0.00000000f,
       0.00000000f,
       0.00000000f,
       0.00000000f,
       0.00000000f,
       0.00000000f,
       0.00000000f,
       0.00000000f,
       0.00000000f,
       0.00000000f,},
    { -0.22500000f,
       0.05000000f,
       0.05000000f,
       0.05000000f,
       0.05000000f,
       0.05000000f,
       0.05000000f,
       0.05000000f,
       0.05000000f,
       0.05000000f,
      -0.22500000f,},
    { -0.45000000f,
      -0.35000000f,
       0.20000000f,
       0.20000000f,
       0.20000000f,
       0.20000000f,
       0.20000000f,
       0.20000000f,
       0.20000000f,
       0.20000000f,
      -0.35000000f,
      -0.45000000f,},
    { -0.67500000f,
      -0.75000000f,
      -0.32500000f,
       0.50000000f,
       0.50000000f,
       0.50000000f,
       0.50000000f,
       0.50000000f,
       0.50000000f,
       0.50000000f,
      -0.32500000f,
      -0.75000000f,
      -0.67500000f,},
};

static float pow_test_cases[][TEST_INPUT_LENGTH] = {
    {  1.00000000f,},
    {  1.00000000f,
       0.11111111f,},
    {  1.00000000f,
       0.08333333f,
       0.08333333f,},
    {  1.00000000f,
       0.08000000f,
       0.04000000f,
       0.08000000f,},
};

static float
rel_err(float *x, float *y, size_t n)
{
    float err;
    size_t i;
    for (i = 0; i < n; i++) {
        x[i] -= y[i];
    }
    err = flt_div(flt_l2norm(x, n), flt_l2norm(y, n));
    return err;
}

#if 0
static void
dump_real(float *x, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) {
        printf("    %16.8f\n", x[i]);
    }
    return;
}

static void
dump_complex(udsp_complex_t *x, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) {
        printf("    %16.8f %16.8f\n", x[i].real, x[i].imag);
    }
    return;
}
#endif

static void
fill_junk(void *x, const size_t n)
{
    const char junk[] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
    char *p = x;
    size_t i;
    for (i = 0; i < n; i++) {
        p[i] = junk[i % sizeof(junk)];
    }
    return;
}

static void
test_fft(void)
{
    udsp_state_t *st;
    size_t i, j, n;
    udsp_complex_t *fft_test;

    st = malloc(sizeof(udsp_state_t));
    if (st == NULL) {
        exit(1);
    }

    for (i = 0; i < N_FFT_TEST_CASES; i++) {
        fft_test = fft_test_cases[i];
        n = i + 1;

        fill_junk(st, sizeof(udsp_state_t));
        udsp_fft_init(st, UDSP_FFT_FFTPACK, n);
        udsp_fft(st, test_input, n, fft_output);
        for (j = 0; j < n; j++) {
            assert(COMPLEX_EQUALS(fft_output[j], fft_test[j]));
        }

        udsp_ifft(st, fft_output, n, test_input);
        for (j = 0; j < n; j++) {
            assert(COMPLEX_EQUALS(fft_output[j], fft_test[j]));
        }
    }

    free(st);

    return;
}

static void
test_fft_shift(void)
{
    udsp_state_t *st;
    size_t i, j, n;
    udsp_complex_t *fft_test;

    st = malloc(sizeof(udsp_state_t));
    if (st == NULL) {
        exit(1);
    }

    for (i = 1; i < N_FFT_TEST_CASES; i++) {
        fft_test = fft_test_cases[i];
        n = i + 1;

        fill_junk(st, sizeof(udsp_state_t));
        udsp_fft_init(st, UDSP_FFT_FFTPACK, n);
        udsp_fft(st, test_input, n, fft_output);
        udsp_fft_shift(fft_output, n);
        udsp_ifft_shift(fft_output, n);

        for (j = 0; j < n; j++) {
            assert(COMPLEX_EQUALS(fft_output[j], fft_test[j]));
        }
    }

    free(st);

    return;
}

static void
test_conv(void)
{
    udsp_state_t *st;
    size_t i, m, n;
    float *conv_test;
    float err;

    st = malloc(2 * sizeof(udsp_state_t));
    if (st == NULL) {
        exit(1);
    }

    for (i = 0; i < N_CONV_TEST_CASES; i++) {
        conv_test = conv_test_cases[i];
        m = TEST_INPUT_LENGTH;
        n = i + 1;

        fill_junk(st, 2 * sizeof(udsp_state_t));
        udsp_conv(st, test_input, m, test_input, n, conv_output);

        err = rel_err(conv_output, conv_test, m + n - 1);
        assert(err < REL_ERR_MAX);
    }

    free(st);

    return;
}

static void
test_xcov(void)
{
    udsp_state_t *st;
    size_t i, m, n;
    float *xcov_test;
    float err;

    st = malloc(2 * sizeof(udsp_state_t));
    if (st == NULL) {
        exit(1);
    }

    for (i = 0; i < N_XCOV_TEST_CASES; i++) {
        xcov_test = xcov_test_cases[i];
        m = TEST_INPUT_LENGTH;
        n = i + 1;

        fill_junk(st, 2 * sizeof(udsp_state_t));
        udsp_xcov(st, test_input, m, test_input, n, xcov_output);

        err = rel_err(xcov_output, xcov_test, m + n - 1);
        assert(err < REL_ERR_MAX);
    }

    free(st);

    return;
}

static void
test_xcor(void)
{
    udsp_state_t *st;
    size_t i, m, n;
    float *xcor_test;
    float err;

    st = malloc(2 * sizeof(udsp_state_t));
    if (st == NULL) {
        exit(1);
    }

    for (i = 0; i < N_XCOR_TEST_CASES; i++) {
        xcor_test = xcor_test_cases[i];
        m = TEST_INPUT_LENGTH;
        n = i + 1;

        fill_junk(st, 2 * sizeof(udsp_state_t));
        udsp_xcor(st, test_input, m, test_input, n, xcor_output);

        err = rel_err(xcor_output, xcor_test, m + n - 1);
        assert(err < REL_ERR_MAX);
    }

    free(st);

    return;
}

static void
test_pow(void)
{
    udsp_state_t *st;
    size_t i, n;
    float *pow_test;
    float err;

    st = malloc(sizeof(udsp_state_t));
    if (st == NULL) {
        exit(1);
    }

    for (i = 0; i < N_POW_TEST_CASES; i++) {
        pow_test = pow_test_cases[i];
        n = i + 1;

        fill_junk(st, sizeof(udsp_state_t));
        udsp_pow(st, test_input, n, pow_output);

        err = rel_err(pow_output, pow_test, n);
        assert(err < REL_ERR_MAX);
    }

    free(st);

    return;
}

typedef void (*test_fn_t)(void);

static test_fn_t test_fns[] = {
    test_fft,
    test_fft_shift,
    test_conv,
    test_xcov,
    test_xcor,
    test_pow,
};

static size_t n_test_fns = sizeof(test_fns) / sizeof(test_fn_t);

int
main(void)
{
    size_t i;
    uint64_t t;

    for (i = 0; i < n_test_fns; i++) {
        nclock_init(&t);
        test_fns[i]();
        nclock_elapsed(&t);
        printf("%" PRIu64 "\n", t);
    }

    return 0;
}
