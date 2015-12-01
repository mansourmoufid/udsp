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

#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include "fltop.h"

/*
 * Word conversion
 */

static inline uint64_t
dtoi(const double x)
{
    union {
        uint64_t i;
        double d;
    } u;
    u.d = x;
    return u.i;
}

static inline double
itod(const uint64_t x)
{
    union {
        uint64_t i;
        double d;
    } u;
    u.i = x;
    return u.d;
}

static inline uint32_t
ftoi(const float x)
{
    union {
        uint32_t i;
        float f;
    } u;
    u.f = x;
    return u.i;
}

static inline float
itof(const uint32_t x)
{
    union {
        uint32_t i;
        float f;
    } u;
    u.i = x;
    return u.f;
}

/*
 * Value testing
 */

/* 01111111 10000000 00000000 00000000 */
#define NANF ((uint32_t) 2139095040ULL)

static inline int
isnanf(const float x)
{
    if ((ftoi(x) & NANF) == NANF) {
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
    return itod(dtoi(x) & SIGND);
}

/* 01111111 11111111 11111111 11111111 */
#define SIGNF ((uint32_t) 2147483647ULL)

static inline float
absf(const float x)
{
    return itof(ftoi(x) & SIGNF);
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

static inline int
sgn(const float x)
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
