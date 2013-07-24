/* Copyright 2013, Mansour Moufid <mansourmoufid@gmail.com>
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

#include <assert.h>
#include <stddef.h>

#include "fltop.h"
#include "udsp.h"

/*
 * Common helper functions
 */

static inline void swap_complex(udsp_complex_t *restrict x,
    udsp_complex_t *restrict y)
{
    udsp_complex_t tmp;
    assert(x != NULL);
    assert(y != NULL);
    tmp.real = x->real;
    tmp.imag = x->imag;
    x->real = y->real;
    x->imag = y->imag;
    y->real = tmp.real;
    y->imag = tmp.imag;
    return;
}

static inline void swap_real(float *restrict x, float *restrict y)
{
    float tmp;
    assert(x != NULL);
    assert(y != NULL);
    tmp = *x;
    *x = *y;
    *y = tmp;
    return;
}

#define DEFINE_REVERSE_FN(NAME, TYPE, SWAP)                 \
        static inline void NAME(TYPE *x, const size_t n)    \
        {                                                   \
            size_t i, l;                                    \
            assert(x != NULL);                              \
            if (n % 2 == 0) {                               \
                l = n / 2;                                  \
            } else {                                        \
                l = (n + 1) / 2;                            \
            }                                               \
            if (l < 1) {                                    \
                return;                                     \
            }                                               \
            for (i = 0; i < l; i++) {                       \
                SWAP(&x[i], &x[(n - 1) - i]);               \
            }                                               \
            return;                                         \
        }

DEFINE_REVERSE_FN(reverse_complex, udsp_complex_t, swap_complex)
DEFINE_REVERSE_FN(reverse_real, float, swap_real)

static void circ_shift_complex(udsp_complex_t *restrict x,
    const size_t n, const size_t l)
{
    assert(x != NULL);
    assert(n > l);
    if (n > 1) {
        if (l > 1) {
            reverse_complex(&x[0], l);
        }
        if (n - l > 1) {
            reverse_complex(&x[l], n - l);
        }
        reverse_complex(x, n);
    }
    return;
}

static inline size_t min(const size_t x, const size_t y)
{
    return (x < y) ? x : y;
}

static inline size_t max(const size_t x, const size_t y)
{
    return (x > y) ? x : y;
}

static inline void copy_real(float *restrict dst,
    const float *restrict src, const size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) {
        dst[i] = src[i];
    }
    return;
}

static inline void copy_complex(udsp_complex_t *restrict dst,
    const udsp_complex_t *restrict src, const size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) {
        dst[i].real = src[i].real;
        dst[i].imag = src[i].imag;
    }
    return;
}

static inline void zero_real(float *restrict x,
    const size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) {
        x[i] = 0.f;
    }
    return;
}

static inline void zero_complex(udsp_complex_t *restrict x,
    const size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) {
        x[i].real = 0.f;
        x[i].imag = 0.f;
    }
    return;
}

static inline void normalize_real(float *restrict x,
    const size_t n, const float denom)
{
    size_t i;
    for (i = 0; i < n; i++) {
        x[i] = flt_div(x[i], denom);
    }
    return;
}

static inline void mul_complex(udsp_complex_t *result,
    const udsp_complex_t *x, const udsp_complex_t *y)
{
    float a, b, c, d;
    assert(result != NULL);
    assert(x != NULL);
    assert(y != NULL);
    a = x->real;
    b = x->imag;
    c = y->real;
    d = y->imag;
    result->real = flt_add(flt_mul(a, c), flt_mul(b, d) * -1.f);
    result->imag = flt_add(flt_mul(a, d), flt_mul(b, c));
    return;
}

#if 0
static void dump_real(float *x, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) {
        printf("    %16.8f\n", x[i]);
    }
    return;
}

static void dump_complex(udsp_complex_t *x, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) {
        printf("    %16.8f %16.8f\n", x[i].real, x[i].imag);
    }
    return;
}
#endif

/*
 * Fast Fourier transform initialization
 */

#if !defined(RFFTI)
#define RFFTI rffti_
#endif
void RFFTI(const size_t *, float *restrict);

static void fftpack_fft_init(struct _udsp_fft_state *restrict st)
{
    assert(st != NULL);
    assert(st->size > 0);
    assert(st->size < UDSP_FFT_SIZE_MAX);
    RFFTI(&(st->size), st->weights);
    return;
}

static void fft_init(udsp_state_t *restrict st,
    const int fft_method, const size_t l,
    const float *restrict x, const size_t n)
{
    struct _udsp_fft_state *fft_st;
    assert(st != NULL);
    assert(fft_method == UDSP_FFT_FFTPACK);
    assert(l > 0);
    assert(l < UDSP_FFT_SIZE_MAX);
    fft_st = &(st->fft_state);
    zero_real(fft_st->rbuf, l);
    zero_complex(fft_st->cbuf, l);
    if (x != NULL) {
        assert(n > 0);
        assert(n < UDSP_FFT_SIZE_MAX);
        copy_real(fft_st->rbuf, x, min(l, n));
    }
    if (fft_st->size == l && fft_st->method == fft_method) {
        return;
    }
    fft_st->size = l;
    fft_st->method = fft_method;
    switch (fft_st->method) {
        case UDSP_FFT_FFTPACK:
            fftpack_fft_init(fft_st);
            break;
        default:
            ;
    }
    return;
}

void udsp_fft_init(udsp_state_t *restrict st,
    const int fft_method, const size_t n)
{
    assert(st != NULL);
    assert(fft_method == UDSP_FFT_FFTPACK);
    assert(n > 0);
    assert(n < UDSP_FFT_SIZE_MAX);
    fft_init(st, fft_method, n, NULL, 0);
    return;
}

/*
 * Fast Fourier transform
 */

#if !defined(UDSP_FFT_DEFAULT)
#define UDSP_FFT_DEFAULT UDSP_FFT_FFTPACK
#endif

size_t udsp_fft_max_size(void)
{
    return UDSP_FFT_SIZE_MAX;
}

#if !defined(RFFTF)
#define RFFTF rfftf_
#endif
void RFFTF(const size_t *, float *restrict, float *restrict);

static void fftpack_unpack(const float *restrict in,
    udsp_complex_t *restrict out, const size_t n)
{
    size_t i, j, l;
    assert(in != NULL);
    assert(out != NULL);
    assert(n > 0);
    if (n % 2 == 0) {
        l = n / 2;
    } else {
        l = (n + 1) / 2;
    }
    out[0].real = in[0];
    out[0].imag = 0.f;
    j = 1;
    for (i = 1; i < l; i++) {
        assert(j + 1 < n);
        out[    i].real = in[j];
        out[    i].imag = in[j + 1];
        out[n - i].real = in[j];
        out[n - i].imag = in[j + 1] * -1.f;
        j += 2;
    }
    if (n % 2 == 0) {
        assert(j < n);
        out[l].real = in[j];
        out[l].imag = 0.f;
    }
    return;
}

static void fftpack_fft(struct _udsp_fft_state *restrict st)
{
    assert(st != NULL);
    assert(st->size > 0);
    assert(st->size < UDSP_FFT_SIZE_MAX);
    RFFTF(&(st->size), st->rbuf, st->weights);
    fftpack_unpack(st->rbuf, st->cbuf, st->size);
    return;
}

void udsp_fft(udsp_state_t *restrict st,
    const float *restrict x, const size_t n,
    udsp_complex_t *restrict result)
{
    struct _udsp_fft_state *fft_st;
    assert(st != NULL);
    fft_st = &(st->fft_state);
    assert(fft_st->method == UDSP_FFT_FFTPACK);
    if (x != NULL) {
        assert(n > 0);
        assert(n < UDSP_FFT_SIZE_MAX);
        zero_real(fft_st->rbuf, fft_st->size);
        copy_real(fft_st->rbuf, x, min(n, fft_st->size));
    }
    switch (fft_st->method) {
        case UDSP_FFT_FFTPACK:
            fftpack_fft(fft_st);
            break;
        default:
            ;
    }
    if (result != NULL) {
        copy_complex(result, fft_st->cbuf, fft_st->size);
    }
    return;
}

/*
 * Inverse fast Fourier transform
 */

#if !defined(RFFTB)
#define RFFTB rfftb_
#endif
void RFFTB(const size_t *, float *restrict, float *restrict);

static void fftpack_pack(const udsp_complex_t *restrict in,
    float *restrict out, const size_t n)
{
    size_t i, j, l;
    assert(in != NULL);
    assert(out != NULL);
    assert(n > 0);
    if (n % 2 == 0) {
        l = n / 2;
    } else {
        l = (n + 1) / 2;
    }
    out[0] = in[0].real;
    j = 1;
    for (i = 1; i < l; i++) {
        assert(j + 1 < n);
        out[j    ] = in[i].real;
        out[j + 1] = in[i].imag;
        j += 2;
    }
    if (n % 2 == 0) {
        out[j] = in[l].real;
    }
    return;
}

static void fftpack_ifft(struct _udsp_fft_state *restrict st)
{
    assert(st != NULL);
    assert(st->size > 0);
    assert(st->size < UDSP_FFT_SIZE_MAX);
    fftpack_pack(st->cbuf, st->rbuf, st->size);
    RFFTB(&(st->size), st->rbuf, st->weights);
    normalize_real(st->rbuf, st->size, (float) st->size);
    return;
}

void udsp_ifft(udsp_state_t *restrict st,
    const udsp_complex_t *restrict x, const size_t n,
    float *restrict result)
{
    struct _udsp_fft_state *fft_st;
    assert(st != NULL);
    fft_st = &(st->fft_state);
    assert(fft_st->method == UDSP_FFT_FFTPACK);
    if (x != NULL) {
        assert(n > 0);
        assert(n < UDSP_FFT_SIZE_MAX);
        zero_complex(fft_st->cbuf, fft_st->size);
        copy_complex(fft_st->cbuf, x, min(n, fft_st->size));
    }
    switch (fft_st->method) {
        case UDSP_FFT_FFTPACK:
            fftpack_ifft(fft_st);
            break;
        default:
            ;
    }
    if (result != NULL) {
        copy_real(result, fft_st->rbuf, fft_st->size);
    }
    return;
}

/*
 * Circular shift
 */

void udsp_fft_shift(udsp_complex_t *restrict x, const size_t n)
{
    assert(x != NULL);
    assert(n > 0);
    if (n % 2 == 0) {
        circ_shift_complex(x, n, n / 2);
    } else {
        circ_shift_complex(x, n, (n + 1) / 2);
    }
    return;
}

void udsp_ifft_shift(udsp_complex_t *restrict x, const size_t n)
{
    assert(x != NULL);
    assert(n > 0);
    if (n % 2 == 0) {
        circ_shift_complex(x, n, n / 2);
    } else {
        circ_shift_complex(x, n, (n - 1) / 2);
    }
    return;
}

/*
 * Convolution
 */

typedef void (*conv_step_t)(udsp_state_t *restrict,
            const float *restrict, const size_t,
            const float *restrict, const size_t,
            float *restrict);

#define CONV_FAMILY_PROTO(NAME)                         \
        void NAME(udsp_state_t st[2],                   \
            const float *restrict x, const size_t m,    \
            const float *restrict y, const size_t n,    \
            float *restrict result)

#define CONV_STEPS_MAX 4

static inline void exec_conv_steps(const conv_step_t steps[],
    udsp_state_t st[2],
    const float *restrict x, const size_t m,
    const float *restrict y, const size_t n,
    float *restrict result)
{
    size_t i;
    assert(steps != NULL);
    for (i = 0; i < CONV_STEPS_MAX; i++) {
        if (steps[i] == NULL) {
            break;
        }
        (*steps[i])(st, x, m, y, n, result);
    }
    return;
}

#define CONV_PRE_FFT     0
#define CONV_POST_FFT    1
#define CONV_PRE_IFFT    2
#define CONV_POST_IFFT   3

static inline void fft_mul(struct _udsp_fft_state *restrict st1,
    struct _udsp_fft_state *restrict st2)
{
    size_t i;
    assert(st1 != NULL);
    assert(st2 != NULL);
    assert(st1->size > 0);
    assert(st1->size < UDSP_FFT_SIZE_MAX);
    assert(st1->size >= st2->size);
    for (i = 0; i < st1->size; i++) {
        mul_complex(&(st1->cbuf[i]),
            &(st1->cbuf[i]), &(st2->cbuf[i]));
    }
    return;
}

static void conv(udsp_state_t st[2],
    const float *restrict x, const size_t m,
    const float *restrict y, const size_t n,
    float *restrict result,
    const conv_step_t steps[][CONV_STEPS_MAX])
{
    size_t l;

    assert(st != NULL);
    assert(x != NULL);
    assert(y != NULL);
    assert(m > 0);
    assert(m < UDSP_FFT_SIZE_MAX);
    assert(n > 0);
    assert(n < UDSP_FFT_SIZE_MAX);
    assert(steps != NULL);

    l = m + n - 1;

    fft_init(&st[0], UDSP_FFT_DEFAULT, l, x, m);
    fft_init(&st[1], UDSP_FFT_DEFAULT, l, y, n);

    exec_conv_steps(steps[CONV_PRE_FFT], st, x, m, y, n, result);
    udsp_fft(&st[0], NULL, 0, NULL);
    udsp_fft(&st[1], NULL, 0, NULL);
    exec_conv_steps(steps[CONV_POST_FFT], st, x, m, y, n, result);

    fft_mul(&(st[0].fft_state), &(st[1].fft_state));

    exec_conv_steps(steps[CONV_PRE_IFFT], st, x, m, y, n, result);
    udsp_ifft(&st[0], NULL, 0, NULL);
    exec_conv_steps(steps[CONV_POST_IFFT], st, x, m, y, n, result);

    if (result != NULL) {
        copy_real(result, st[0].fft_state.rbuf, l);
    }

    return;
}

CONV_FAMILY_PROTO(udsp_conv)
{
    assert(st != NULL);
    assert(x != NULL);
    assert(m > 0);
    assert(m < UDSP_FFT_SIZE_MAX);
    assert(n > 0);
    assert(n < UDSP_FFT_SIZE_MAX);
    assert(result != NULL);
    conv(st, x, m, y, n, result,
        (const conv_step_t [][CONV_STEPS_MAX]) {
            [CONV_PRE_FFT] = {NULL},
            [CONV_POST_FFT] = {NULL},
            [CONV_PRE_IFFT] = {NULL},
            [CONV_POST_IFFT] = {NULL},
        }
    );
    return;
}

/*
 * Cross-covariance
 */

CONV_FAMILY_PROTO(time_domain_reverse)
{
    assert(st != NULL);
    assert(st[1].fft_state.rbuf != NULL);
    assert(n > 0);
    assert(n < UDSP_FFT_SIZE_MAX);
    (void) x;
    (void) y;
    (void) m;
    (void) result;
    reverse_real(st[1].fft_state.rbuf, n);
    return;
}

CONV_FAMILY_PROTO(time_domain_normalize)
{
    assert(st != NULL);
    assert(st[0].fft_state.rbuf != NULL);
    assert(m > 0);
    assert(m < UDSP_FFT_SIZE_MAX);
    assert(n > 0);
    assert(n < UDSP_FFT_SIZE_MAX);
    (void) x;
    (void) y;
    (void) result;
    normalize_real(st[0].fft_state.rbuf, st[0].fft_state.size,
        max(m, n));
    return;
}

CONV_FAMILY_PROTO(udsp_xcov)
{
    assert(st != NULL);
    assert(x != NULL);
    assert(m > 0);
    assert(m < UDSP_FFT_SIZE_MAX);
    assert(n > 0);
    assert(n < UDSP_FFT_SIZE_MAX);
    assert(result != NULL);
    conv(st, x, m, y, n, result,
        (const conv_step_t [][CONV_STEPS_MAX]) {
            [CONV_PRE_FFT] = {
                &time_domain_reverse,
                NULL
            },
            [CONV_POST_FFT] = {NULL},
            [CONV_PRE_IFFT] = {NULL},
            [CONV_POST_IFFT] = {
                &time_domain_normalize,
                NULL
            },
        }
    );
    return;
}

/*
 * Cross-correlation
 */

static inline void demean_real(float *restrict x, const size_t n)
{
    float mean;
    size_t i;
    mean = flt_div(flt_sum(x, n), (float) n);
    for (i = 0; i < n; i++) {
        x[i] = flt_add(x[i], -mean);
    }
    return;
}

CONV_FAMILY_PROTO(time_domain_demean)
{
    assert(st != NULL);
    assert(m > 0);
    assert(m < UDSP_FFT_SIZE_MAX);
    assert(n > 0);
    assert(n < UDSP_FFT_SIZE_MAX);
    (void) x;
    (void) y;
    (void) result;
    demean_real(st[0].fft_state.rbuf, m);
    demean_real(st[1].fft_state.rbuf, n);
    return;
}

CONV_FAMILY_PROTO(udsp_xcor)
{
    conv(st, x, m, y, n, result,
        (const conv_step_t [][CONV_STEPS_MAX]) {
            [CONV_PRE_FFT] = {
                &time_domain_demean,
                &time_domain_reverse,
                NULL
            },
            [CONV_POST_FFT] = {NULL},
            [CONV_PRE_IFFT] = {NULL},
            [CONV_POST_IFFT] = {
                &time_domain_normalize,
                NULL
            },
        }
    );
    return;
}

/*
 * Periodogram
 */

static void square_complex(udsp_complex_t *result,
    const udsp_complex_t *x)
{
    float a, b;
    a = x->real;
    b = x->imag;
    result->real = flt_add(flt_mul(a, a), flt_mul(b, b));
    result->imag = 0.f;
    return;
}

static inline void fft_square(struct _udsp_fft_state *restrict st)
{
    size_t i;
    assert(st != NULL);
    assert(st->size > 0);
    assert(st->size < UDSP_FFT_SIZE_MAX);
    for (i = 0; i < st->size; i++) {
        square_complex(&(st->cbuf[i]), &(st->cbuf[i]));
    }
    return;
}

void udsp_pow(udsp_state_t *st,
    const float *restrict x, const size_t n,
    float *restrict result)
{
    size_t i;
    float pow_max;

    assert(st != NULL);
    assert(x != NULL);
    assert(n > 0);
    assert(n < UDSP_FFT_SIZE_MAX);

    fft_init(st, UDSP_FFT_DEFAULT, n, x, n);
    udsp_fft(st, NULL, 0, NULL);
    fft_square(&(st->fft_state));
    pow_max = st->fft_state.cbuf[0].real;
    normalize_real((float *) st->fft_state.cbuf, 2 * n, pow_max);

    if (result != NULL) {
        for (i = 0; i < n; i++) {
            result[i] = st->fft_state.cbuf[i].real;
        }
    }

    return;
}
