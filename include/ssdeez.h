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

#ifndef SSDEEZ_H
#define SSDEEZ_H

#ifdef __cplusplus
extern "C" {
#endif  // `__cplusplus`

/* Includes ==============================================================> */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Macros =================================================================> */

// clang-format off

/* Compiler-specific attribute for a function that must be always-inlined. */
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

    #define DZ_API_STATIC_INLINE static DZ_API_INLINE
#endif  // `DZ_API_INLINE`

/* Prints the "not implemented" message and aborts this program. */
#define DZ_API_UNIMPLEMENTED()              \
    do {                                    \
        fprintf(                            \
            stderr,                         \
            __FILE__                        \
            ":%d: not implemented: %s\n",   \
            __LINE__, __func__              \
        );                                  \
                                            \
        abort();                            \
    } while (0)                             \

/* Suppresses the "unused parameter/variable" errors. */
#define DZ_API_UNUSED_VARIABLE(x)  ((void) (x))

/* Represents the current API version of SSDeez. */
#define DZ_API_VERSION             "0.0.2"

/* Macro-defined Constants ================================================> */

// TODO: ...

/* Typedefs ===============================================================> */

/* Aliases for primitive types. */

typedef bool           dzBool;

typedef unsigned char  dzByte;

typedef ptrdiff_t      dzISize;
typedef size_t         dzUSize;

typedef int16_t        dzI16;
typedef int32_t        dzI32;
typedef int64_t        dzI64;

typedef uint16_t       dzU16;
typedef uint32_t       dzU32;
typedef uint64_t       dzU64;

typedef float          dzF32;
typedef double         dzF64;

/* ========================================================================> */

/* Represents the identifier of a page, a block, a plane, a die, or a chip. */
typedef uint64_t       dzID;

// clang-format on

/* ========================================================================> */

/* An enumeration that represents the type of a NAND flash cell. */
typedef enum dzCellType_ {
    DZ_CELL_UNKNOWN = -1,
    DZ_CELL_SLC,  // 2 voltage states
    DZ_CELL_MLC,  // 4 voltage states
    DZ_CELL_TLC,  // 8 voltage states
    DZ_CELL_QLC,  // 16 voltage states
    DZ_CELL_COUNT_
} dzCellType;

/* An enumeration that represents the error codes returned by functions. */
typedef enum dzResult_ {
    DZ_RES_OK = 0,
    DZ_RES_INVALID_ARGUMENT,  // Invalid IDs, `NULL` pointers, etc.
    DZ_RES_OUT_OF_MEMORY,     // `malloc()` or `calloc()` returned `NULL`.
    DZ_RES_UNKNOWN,           // Oopsie?
    DZ_RES_COUNT_
} dzResult;

/* ========================================================================> */

/* A structure that represents a NAND flash die. */
typedef struct dzDie_ dzDie;

/* A structure that represents the configuration of a NAND flash die. */
typedef struct dzDieConfig_ {
    dzID dieId;
    dzCellType cellType;
    dzF32 badBlockRatio;
    dzU32 planeCountPerDie;
    dzU32 blockCountPerPlane;
    dzU32 pageCountPerBlock;
    dzU32 pageSizeInBytes;
} dzDieConfig;

/* NOTE: Assuming each logical unit (LUN) consists of only one die? */
typedef dzDie dzLUN;

/* ========================================================================> */

/* A structure that represents a NAND flash chip, also known as a 'target'. */
typedef struct dzChip_ dzChip;

/* A structure that represents the configuration of a NAND flash chip. */
typedef struct dzChipConfig_ {
    dzDieConfig *dieConfig;
    dzID chipId;
    dzU32 dieCount;
} dzChipConfig;

/* ========================================================================> */

/* A structure that represents a byte array. */
typedef struct dzByteArray_ {
    dzByte *ptr;
    dzUSize size;
} dzByteArray;

/* A structure that represents a byte stream with a position indicator. */
typedef struct dzByteStream_ {
    dzByte *ptr;
    dzUSize size;
    dzUSize offset;
} dzByteStream;

/* Constants ==============================================================> */

/* A constant that represents an invalid identifier. */
extern const dzID DZ_API_INVALID_ID;

/* Public Functions =======================================================> */

/* <---------------------------------------------------------- [src/block.c] */

// TODO: ...

/* <----------------------------------------------------------- [src/chip.c] */

/* Initializes `*chip` with the given `config`. */
dzResult dzChipInit(dzChip **chip, dzChipConfig config);

/* Releases the memory allocated for `chip`. */
void dzChipDeinit(dzChip *chip);

/* Public Functions =======================================================> */

// TODO: dzChipBlockErase()

// TODO: dzChipChangeReadColumn()

// TODO: dzChipChangeWriteColumn()

// TODO: dzChipCopyback()

// TODO: dzChipGetFeatures()

// TODO: dzChipPageProgram()

// TODO: dzChipRead()

// TODO: dzChipReadCache()

// TODO: dzChipReadID()

// TODO: dzChipReadParameterPage()

// TODO: dzChipReadStatus()

// TODO: dzChipReadStatusEnhanced()

// TODO: dzChipReadUniqueID()

/* Performs a "Reset" operation. */
dzResult dzChipReset(dzChip *chip);

// TODO: dzChipSetFeatures()

/* <------------------------------------------------------------ [src/die.c] */

/* Initializes `*die` with the given `config`. */
dzResult dzDieInit(dzDie **die, dzDieConfig config);

/* Releases the memory allocated for `die`. */
void dzDieDeinit(dzDie *die);

/* <----------------------------------------------------------- [src/onfi.c] */

// TODO: ...

/* <----------------------------------------------------------- [src/page.c] */

// TODO: ...

/* <---------------------------------------------------------- [src/plane.c] */

// TODO: ...

/* <---------------------------------------------------------- [src/utils.c] */

/* Returns a pseudo-random floating-point value from a Gaussian distribution. */
dzF64 dzUtilsGaussian(dzF64 mu, dzF64 sigma);

/* Returns a pseudo-random unsigned 64-bit value. */
dzU64 dzUtilsRand(void);

/* 
    Returns a pseudo-random unsigned integer
    in the given (inclusive) range.
*/
dzU64 dzUtilsRandRange(dzU64 min, dzU64 max);

/* 
    Returns a pseudo-random floating-point value
    in the given (exclusive) range.
*/
dzF64 dzUtilsRandRangeF64(dzF64 min, dzF64 max);

/* Seeds the pseudo-random number generator. */
void dzUtilsSrand(dzU64 seed);

/* Inline Functions =======================================================> */

/* Returns `value` clamped to the inclusive range of `low` and `high`. */
DZ_API_INLINE dzF64 dzUtilsClampF64(dzF64 value, dzF64 low, dzF64 high) {
    if (low > high) {
        dzF64 temp = low;

        low = high, high = temp;
    }

    return (value >= low) ? ((value <= high) ? value : high) : low;
}

/* ========================================================================> */

#ifdef __cplusplus
}
#endif  // `__cplusplus`

#endif  // `SSDEEZ_H`
