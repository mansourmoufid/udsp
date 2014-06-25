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

#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include "fltop.h"

/*
 * Value testing
 */

/* 01111111 11110000 00000000 00000000 ... */
#define NAND ((uint64_t) 9218868437227405312ULL)

static inline int
isnand(const double x)
{
    uint64_t o;
    o = *((const uint64_t *) &x);
    if ((o & NAND) == NAND) {
        return 1;
    }
    return 0;
}

static int
isreald(const double x)
{
    return !isnand(x);
}

/* 01111111 10000000 00000000 00000000 */
#define NANF ((uint32_t) 2139095040ULL)

static inline int
isnanf(const float x)
{
    uint32_t q;
    q = *((const uint32_t *) &x);
    if ((q & NANF) == NANF) {
        return 1;
    }
    return 0;
}

static int
isrealf(const float x)
{
    return !isnanf(x);
}

int
flt_isreal(const float x)
{
    return !isnanf(x);
}

/* 01111111 11111111 11111111 11111111 ... */
#define SIGND ((uint64_t) 9223372036854775807ULL)

static inline double
absd(const double x)
{
    uint64_t o;
    o = *((uint64_t *) &x);
    o = o & SIGND;
    return *((double *) &o);
}

/* 01111111 11111111 11111111 11111111 */
#define SIGNF ((uint32_t) 2147483647ULL)

static inline float
absf(const float x)
{
    uint32_t q;
    q = *((uint32_t *) &x);
    q = q & SIGNF;
    return *((float *) &q);
}

float
flt_abs(const float x)
{
    return absf(x);
}

static inline int
iszerod(const double x)
{
    return (absd(x) < (double) FLTOP_EPS);
}

static inline int
iszerof(const float x)
{
    return (absf(x) < (float) FLTOP_EPS);
}

int
flt_iszero(const float x)
{
    return (isrealf(x) && iszerof(x));
}

/*
 * Relational operators
 */

#define DECL_COND_FN(COND, COND_EXPR)                   \
        int flt_ ##COND (const float x, const float y)  \
        {                                               \
            double delta;                               \
            if (!isrealf(x) || !isrealf(y)) {           \
                return 0;                               \
            }                                           \
            delta = (double) x - (double) y;            \
            return ((COND_EXPR));                       \
        }

DECL_COND_FN(eq, iszerod(delta))
DECL_COND_FN(ne, !iszerod(delta))
DECL_COND_FN(ge, delta > 0. || iszerod(delta))
DECL_COND_FN(gt, delta > 0. && !iszerod(delta))
DECL_COND_FN(le, delta < 0. || iszerod(delta))
DECL_COND_FN(lt, delta < 0. && !iszerod(delta))

/*
 * Floating-point arithmetic
 */

static inline int sgn(const float x)
{
    if (!isrealf(x)) {
        return 0;
    } else if (iszerof(x)) {
        return 0;
    } else if (x < 0.f) {
        return -1;
    } else {
        return 1;
    }
}

static inline float
fsgn(const float x)
{
    return (float) sgn(x);
}

int
flt_sgn(const float x)
{
    return sgn(x);
}

static inline float
dtof(const double x)
{
    if (!isreald(x)) {
        return FLTOP_NAN_RETURN;
    }
    if (iszerod(x)) {
        return 0.f;
    } else if (x > (double) FLT_MAX - FLTOP_EPS) {
        return (FLT_MAX * fsgn(x));
    } else {
        return ((float) x);
    }
}

float
flt_add(const float x, const float y)
{
    if (!isrealf(x) || !isrealf(y)) {
        return FLTOP_NAN_RETURN;
    }
    if (sgn(x) == sgn(y)) {
        if (absf(x) < FLT_MAX - absf(y)) {
            return (x + y);
        } else {
            return (FLT_MAX * fsgn(x));
        }
    } else {
        return (x + y);
    }
}

float
flt_mul(const float x, const float y)
{
    if (!isrealf(x) || !isrealf(y)) {
        return FLTOP_NAN_RETURN;
    }
    if (iszerof(y)) {
        return 0.f;
    } else if (absf(y) < 1.f || absf(x) < FLT_MAX / absf(y)) {
        return (x * y);
    } else {
        return (FLT_MAX * fsgn(x) * fsgn(y));
    }
}

static inline float
inv(const float x)
{
    if (iszerof(x)) {
        if (x < 0.f) {
            return -FLT_MAX;
        } else {
            return FLT_MAX;
        }
    } else if (absf(x) >= 1.f / FLTOP_EPS) {
        return 0.f;
    } else {
        return (1.f / x);
    }
}

float
flt_div(const float x, const float y)
{
    if (!isrealf(x) || !isrealf(y)) {
        return FLTOP_NAN_RETURN;
    }
    return flt_mul(x, inv(y));
}

float
flt_sum(const float *restrict x, const size_t n)
{
    float sum;
    size_t i;
    sum = 0.f;
    for (i = 0; i < n; i++) {
        if (isrealf(x[i])) {
            sum = flt_add(sum, x[i]);
        }
    }
    return sum;
}

float
flt_l2norm(const float *restrict x, const size_t n)
{
    float square, sum, norm;
    size_t i;
    sum = 0.f;
    for (i = 0; i < n; i++) {
        if (isrealf(x[i])) {
            square = flt_mul(x[i], x[i]);
            sum = flt_add(sum, square);
        }
    }
    norm = sqrtf(sum);
    return norm;
}
