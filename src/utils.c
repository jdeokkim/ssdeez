/*
    Copyright (c) 2025 Jaedeok Kim (jdeokkim@protonmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

/* Includes ===============================================================> */

#include <float.h>
#include <limits.h>
#include <math.h>

#include "ssdeez.h"

/* Macros =================================================================> */

#define BITWISE_ROTATE_LEFT_U64(x, k) (((x) << (k)) | ((x) >> (64 - (k))))

/* Typedefs ===============================================================> */

// TODO: ...

/* Private Function Prototypes ============================================> */

/* Returns the next pseudo-random number from the xoshiro256++ generator. */
static DZ_API_INLINE dzU64 dzUtilsRand(void);

/* Constants ==============================================================> */

/* 
    Special constant for `dzUtilsGaussian()`, 
    in order to avoid explicit `float` conversions. 
*/
static const dzF32 INVERSE_UINT64_MAX = 1.0f / UINT64_MAX;

/* Private Variables ======================================================> */

/* NOTE: Deterministic behavior for research purposes? */
static dzU64 prngStates[4] = { 0x2025, 0x0926, 0x2116, 0x1200 };

/* Public Functions =======================================================> */

/* Returns a pseudo-random number from a Gaussian distribution. */
dzF32 dzUtilsGaussian(dzF32 mu, dzF32 sigma) {
    /* 
        NOTE: Marsaglia's polar method, with pseudo-random numbers 
              from the xoshiro256++ generator.
    */

    static dzBool hasNextValue = false;

    static dzF32 nextValue = FLT_EPSILON;

    if (hasNextValue) {
        hasNextValue = !hasNextValue;

        return mu + (nextValue * sigma);
    } else {
        dzF32 x, y, r;

        do {
            x = 2.0f * (dzUtilsRand() * INVERSE_UINT64_MAX) - 1.0f;
            y = 2.0f * (dzUtilsRand() * INVERSE_UINT64_MAX) - 1.0f;

            r = x * x + y * y;
        } while (r >= 1.0f || r == 0.0f);

        r = sqrtf(-2.0f * (logf(r) / r));

        nextValue = y * r, hasNextValue = !hasNextValue;

        return mu + ((x * r) * sigma);
    }
}

/* Private Functions ======================================================> */

/* Returns the next pseudo-random number from the xoshiro256++ generator. */
static DZ_API_INLINE dzU64 dzUtilsRand(void) {
    /*
        NOTE: The original code was written in 2019
              by David Blackman and Sebastiano Vigna.

              (https://prng.di.unimi.it)
    */

    dzU64 result = BITWISE_ROTATE_LEFT_U64(prngStates[0] + prngStates[3], 23)
                   + prngStates[0];

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
