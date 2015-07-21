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

#if !defined(FLTOP_H)
#define FLTOP_H

#include <float.h>
#include <stddef.h>

#if !defined(FLTOP_EPS)
#define FLTOP_EPS 1.19209290e-5f
#endif

#if !defined(FLTOP_NAN_RETURN)
#define FLTOP_NAN_RETURN 0.f
#endif

int flt_isreal(const float);
int flt_iszero(const float);
int flt_eq(const float, const float);
int flt_ne(const float, const float);
int flt_ge(const float, const float);
int flt_gt(const float, const float);
int flt_le(const float, const float);
int flt_lt(const float, const float);
int flt_sgn(const float);
float flt_abs(const float);
float flt_add(const float, const float);
float flt_mul(const float, const float);
float flt_div(const float, const float);
float flt_sum(const float *restrict, const size_t);
float flt_l2norm(const float *restrict x, const size_t n);

#endif
