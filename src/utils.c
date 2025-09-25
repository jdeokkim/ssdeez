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

#include "ssdeez.h"

/* Macros =================================================================> */

#define BITWISE_ROTATE_LEFT_U64(x, k) \
    (((x) << (k)) | ((x) >> (64 - (k))))

/* Typedefs ===============================================================> */

// TODO: ...

/* Private Function Prototypes ============================================> */

// TODO: ...

/* Constants ==============================================================> */

// TODO: ...

/* Private Variables ======================================================> */

/* NOTE: Deterministic behavior for research purposes... */
static dzU64 prngState[4] = { 0x20, 0x25, 0x09, 0x26 };

/* Public Functions =======================================================> */

/* Returns the next pseudo-random number from the xoshiro256++ generator. */
dzU64 dzUtilsRand(void) {
    /*
        NOTE: The original code was written in 2019
              by David Blackman and Sebastiano Vigna.

              (https://prng.di.unimi.it)
    */

    dzU64 result = BITWISE_ROTATE_LEFT_U64(prngState[0] + prngState[3], 23) \
        + prngState[0];

	{
        const dzU64 tempValue = prngState[1] << 17;

        prngState[2] ^= prngState[0];
        prngState[3] ^= prngState[1];
        prngState[1] ^= prngState[2];
        prngState[0] ^= prngState[3];

        prngState[2] ^= tempValue;

        prngState[3] = BITWISE_ROTATE_LEFT_U64(prngState[3], 45);
    }

	return result;
}

/* Private Functions ======================================================> */

// TODO: ...
