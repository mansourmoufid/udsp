/* Copyright 2013, 2014, Mansour Moufid <mansourmoufid@gmail.com>
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

#if !defined(UDSP_H)
#define UDSP_H

#include <stddef.h>

struct udsp_complex {
    float real;
    float imag;
};

typedef struct udsp_complex udsp_complex_t;

#if !defined(UDSP_FFT_SIZE_MAX)
#define UDSP_FFT_SIZE_MAX (64 * 1024)
#endif

struct _udsp_fft_state {
    float weights[2 * UDSP_FFT_SIZE_MAX + 16];
    float rbuf[2 * UDSP_FFT_SIZE_MAX];
    udsp_complex_t cbuf[2 * UDSP_FFT_SIZE_MAX];
    size_t size;
    int method;
};

struct udsp_state {
    struct _udsp_fft_state fft_state;
    char pad[64 - sizeof(struct _udsp_fft_state) % 64];
};

typedef struct udsp_state udsp_state_t;

#define UDSP_FFT_FFTPACK 1

void udsp_fft_init(udsp_state_t *restrict,
    const int, const size_t);

void udsp_fft(udsp_state_t *restrict,
    const float *restrict, const size_t, udsp_complex_t *restrict);

void udsp_ifft(udsp_state_t *restrict,
    const udsp_complex_t *restrict, const size_t, float *restrict);

void udsp_fft_shift(udsp_complex_t *restrict, const size_t);

void udsp_ifft_shift(udsp_complex_t *restrict, const size_t);

#define CONV_FAMILY_DECL(NAME)                      \
        void NAME(udsp_state_t *restrict,           \
            const float *restrict, const size_t,    \
            const float *restrict, const size_t,    \
            float *restrict);

CONV_FAMILY_DECL(udsp_conv)
CONV_FAMILY_DECL(udsp_xcov)
CONV_FAMILY_DECL(udsp_xcor)

#undef CONV_FAMILY_DECL

void udsp_pow(udsp_state_t *,
    const float *restrict, const size_t, float *restrict);

#endif
