/*
    Copyright (c) 2025 Jaedeok Kim <jdeokkim@protonmail.com>

    Permission is hereby granted, free of charge, to any person obtaining a 
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation 
    the rights to use, copy, modify, merge, publish, distribute, sublicense, 
    and/or sell copies of the Software, and to permit persons to whom the 
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included 
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.
*/

/* Includes ===============================================================> */

#include <limits.h>
#include <math.h>

#include "ssdeez.h"

/* Macros =================================================================> */

#define BITWISE_ROTATE_LEFT_U64(x, k) (((x) << (k)) | ((x) >> (64 - (k))))

/* Typedefs ===============================================================> */

// TODO: ...

/* Constants ==============================================================> */

// TODO: ...

/* Private Variables ======================================================> */

/* NOTE: Deterministic behavior for research purposes? */
static dzU64 prngStates[4] = { 0x2025, 0x0926, 0x2116, 0x1200 };

/* Private Function Prototypes ============================================> */

/* Returns the next pseudo-random number from the xoshiro256+ generator. */
DZ_API_STATIC_INLINE dzU64 dzUtilsXoshiroPlus(void);

/* Returns a pseudo-random number from an uniform distribution. */
DZ_API_STATIC_INLINE dzF64 dzUtilsUniform(void);

/* Public Functions =======================================================> */

/* Returns a pseudo-random number from a Gaussian distribution. */
dzF64 dzUtilsGaussian(dzF64 mu, dzF64 sigma) {
    /* 
        NOTE: Marsaglia's polar method, with pseudo-random numbers 
              from the xoshiro256++ generator.
    */

    static dzBool hasNextValue = false;

    static dzF64 nextValue = 0.0;

    if (hasNextValue) {
        hasNextValue = !hasNextValue;

        return mu + (nextValue * sigma);
    } else {
        dzF64 x, y, r;

        do {
            x = (2.0 * dzUtilsUniform()) - 1.0;
            y = (2.0 * dzUtilsUniform()) - 1.0;

            r = x * x + y * y;
        } while (r >= 1.0 || r == 0.0);

        r = sqrt((-2.0 * log(r)) / r);

        nextValue = y * r, hasNextValue = !hasNextValue;

        return mu + ((x * r) * sigma);
    }
}

/* Returns a pseudo-random unsigned 64-bit integer. */
dzU64 dzUtilsRand(void) {
    return dzUtilsXoshiroPlus();
}

/* 
    Returns a pseudo-random unsigned 64-bit integer 
    in the given (inclusive) range. 
*/
dzU64 dzUtilsRandRange(dzU64 min, dzU64 max) {
    return (min < max)
               ? (min + (dzU64) ((dzF64) (max - min + 1) * dzUtilsUniform()))
               : (max + (dzU64) ((dzF64) (min - max + 1) * dzUtilsUniform()));
}

/* 
    Returns a pseudo-random double-precision floating-point number 
    in the given (exclusive) range. 
*/
dzF64 dzUtilsRandRangeF64(dzF64 min, dzF64 max) {
    return (min < max) ? (min + ((max - min) * dzUtilsUniform()))
                       : (max + ((min - max) * dzUtilsUniform()));
}

/* Private Functions ======================================================> */

/* Returns the next pseudo-random number from the xoshiro256+ generator. */
DZ_API_STATIC_INLINE dzU64 dzUtilsXoshiroPlus(void) {
    /*
        NOTE: The original code was written in 2019
              by David Blackman and Sebastiano Vigna.

              (https://prng.di.unimi.it)
    */

    dzU64 result = prngStates[0] + prngStates[3];

    {
        const dzU64 tempValue = prngStates[1] << 17;

        prngStates[2] ^= prngStates[0];
        prngStates[3] ^= prngStates[1];
        prngStates[1] ^= prngStates[2];
        prngStates[0] ^= prngStates[3];

        prngStates[2] ^= tempValue;

        prngStates[3] = BITWISE_ROTATE_LEFT_U64(prngStates[3], 45);
    }

    return result;
}

/* Returns a pseudo-random number from an uniform distribution. */
DZ_API_STATIC_INLINE dzF64 dzUtilsUniform(void) {
    // NOTE: Extract the upper 53 bits, then multiply by 2^(-53)
    return 1.11022302462515654e-16 * ((dzF64) (dzUtilsXoshiroPlus() >> 11));
}
