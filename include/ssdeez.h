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

#ifndef SSDEEZ_H
#define SSDEEZ_H

#ifdef __cplusplus
extern "C" {
#endif  // `__cplusplus`

/* Includes ==============================================================> */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* Macros =================================================================> */

/* Compiler-specific attribute for a function that must be inlined. */
#ifndef DZ_API_INLINE
    #ifdef _MSC_VER
        #define DZ_API_INLINE __forceinline
    #elif defined(__GNUC__)
        #if defined(__STRICT_ANSI__)
            #define DZ_API_INLINE __inline__ __attribute__((always_inline))
        #else
            #define DZ_API_INLINE inline __attribute__((always_inline))
        #endif
    #else
        #define DZ_API_INLINE inline
    #endif
#endif  // `DZ_API_INLINE`

/* Typedefs ===============================================================> */

/* Aliases for primitive integer types. */

typedef bool          dzBool;

typedef unsigned char dzByte;

typedef int32_t       dzI32;
typedef int64_t       dzI64;

typedef uint32_t      dzU32;
typedef uint64_t      dzU64;

typedef float         dzF32;
typedef double        dzF64;

/* ========================================================================> */

/* An enumeration that represents the type of a NAND flash cell. */
typedef enum dzCellType_ {
    DZ_CELL_TYPE_UNKNOWN,
    DZ_CELL_TYPE_SLC,      // 1 voltage state
    DZ_CELL_TYPE_MLC,      // 2 voltage states
    DZ_CELL_TYPE_TLC,      // 3 voltage states
    DZ_CELL_TYPE_QLC,      // 4 voltage states
    DZ_CELL_TYPE_COUNT_
} dzCellType;

/* ========================================================================> */

/* 
    A structure that represents an interface between the controller 
    and a group of dies. 
*/
typedef struct dzChannel_ dzChannel;

/* ========================================================================> */

/* A structure that represents a group of NAND flash planes. */
typedef struct dzDie_ dzDie;

/* A structure that represents the configuration of a `dzDie`. */
typedef struct dzDieConfig_ {
    dzCellType cellType;
    dzU32 planeCountPerDie;
    dzU32 blockCountPerPlane;
    dzU32 layerCountPerBlock;
    dzU32 pageSizeInBytes;
} dzDieConfig;

/* Public Functions =======================================================> */

/* <-------------------------------------------------------- [src/channel.c] */

/* Creates a channel with the given `id`. */
dzChannel *dzChannelCreate(dzI32 id);

/* Releases the memory allocated for the `channel`. */
void dzChannelRelease(dzChannel *channel);

/* <------------------------------------------------------------ [src/die.c] */

/* Creates a die with the given configuration. */
dzDie *dzDieCreate(dzDieConfig config);

/* Releases the memory allocated for the `die`. */
void dzDieRelease(dzDie *die);

// TODO: dzDieProgramPage

// TODO: dzDieReadPage

/* <---------------------------------------------------------- [src/utils.c] */

/* Returns a pseudo-random number from a Gaussian distribution. */
dzF32 dzUtilsGaussian(dzF32 mu, dzF32 sigma);

/* ========================================================================> */

#ifdef __cplusplus
}
#endif  // `__cplusplus`

#endif  // `SSDEEZ_H`
