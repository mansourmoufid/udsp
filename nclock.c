/* Copyright 2014, Mansour Moufid <mansourmoufid@gmail.com>
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

#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200112L
#endif

#include <assert.h>
#include <stdint.h>
#if defined(__linux__)
#include <time.h>
#endif

#include "nclock.h"

#if defined(__linux__)
static const clockid_t clock_ids[] = {
#if defined(CLOCK_MONOTONIC_RAW)
    CLOCK_MONOTONIC_RAW,
#endif
#if defined(CLOCK_MONOTONIC)
    CLOCK_MONOTONIC,
#endif
    CLOCK_REALTIME,
};
static const size_t n_clock_ids = sizeof(clock_ids) / sizeof(clockid_t);
static int
nclock_init_linux(uint64_t *timer)
{
    struct timespec ts;
    size_t i;
    assert(timer != NULL);
    for (i = 0; i < n_clock_ids; i++) {
        if (clock_gettime(clock_ids[i], &ts) == 0) {
            goto success;
        }
    }
    return 1;
success:
    *timer = (uint64_t) ts.tv_sec;
    *timer *= 1000000000;
    *timer += (uint64_t) ts.tv_nsec;
    return 0;
}
#endif

int
nclock_init(uint64_t *timer)
{
#if defined(__linux__)
    return nclock_init_linux(timer);
#else
    (void) timer;
    return 1;
#endif
}

int
nclock_elapsed(uint64_t *timer)
{
    uint64_t now;
    assert(timer != NULL);
    if (nclock_init(&now) != 0) {
        return 1;
    }
    assert(now >= *timer);
    *timer = now - *timer;
    return 0;
}
