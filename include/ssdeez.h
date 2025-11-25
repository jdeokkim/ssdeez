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

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Macros =================================================================> */

// clang-format off

/* Compiler-specific attribute for a function that must be always-inlined. */
#ifndef DZ_API_INLINE
    #if defined(_MSC_VER)
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
#define DZ_API_UNIMPLEMENTED()                 \
    do {                                       \
        fprintf(                               \
            stderr,                            \
            __FILE__                           \
            ":%d: not implemented: %s()\n\n",  \
            __LINE__, __func__                 \
        );                                     \
                                               \
        abort();                               \
    } while (0)                                \

/* Suppresses the "unused parameter/variable" errors. */
#define DZ_API_UNUSED_VARIABLE(x)  ((void) (x))

/* ------------------------------------------------------------------------> */

/* Logs a message with the given `level`. */
#define DZ_API_LOG(level, ...)                        \
    do {                                              \
        fprintf(stderr, level ": %s(): ", __func__),  \
        fprintf(stderr, __VA_ARGS__);                 \
    } while (0)                                       \

/* Logs a message with the 'DEBUG' level. */
#define DZ_API_DEBUG(...)    DZ_API_LOG("DEBUG", __VA_ARGS__);

/* Logs a message with the 'ERROR' level. */
#define DZ_API_ERROR(...)    DZ_API_LOG("ERROR", __VA_ARGS__);

/* Logs a message with the 'INFO' level. */
#define DZ_API_INFO(...)     DZ_API_LOG("INFO", __VA_ARGS__);

/* Logs a message with the 'WARNING' level. */
#define DZ_API_WARNING(...)  DZ_API_LOG("WARNING", __VA_ARGS__);

/* ------------------------------------------------------------------------> */

/* Represents the current API version of SSDeez. */
#define DZ_API_VERSION       "0.0.1"

/* Typedefs ===============================================================> */

/* Aliases for primitive types. */

typedef bool dzBool;

typedef unsigned char dzByte;

typedef ptrdiff_t dzISize;
typedef size_t dzUSize;

typedef int16_t dzI16;
typedef int32_t dzI32;
typedef int64_t dzI64;

typedef uint16_t dzU16;
typedef uint32_t dzU32;
typedef uint64_t dzU64;

typedef float dzF32;
typedef double dzF64;

typedef char *dzString;

/* ------------------------------------------------------------------------> */

/* Timestamp value, in microseconds. */
typedef dzU64 dzTimestamp;

/* The identifier of a page, a block, a plane, a die, or a chip. */
typedef dzU64 dzID;

// clang-format on

/* ------------------------------------------------------------------------> */

/* An enumeration that represents the error codes returned by functions. */
typedef enum dzResult_ {
    DZ_RESULT_OK = 0,
    DZ_RESULT_INJECTION_FAILED,  // Failed to inject bad blocks into a die
    DZ_RESULT_INVALID_ARGUMENT,  // Invalid IDs, `NULL` pointers, etc.
    DZ_RESULT_OUT_OF_MEMORY,     // `malloc()` or `calloc()` returned `NULL`
    DZ_RESULT_UNKNOWN,           // Oopsie?
    DZ_RESULT_COUNT_
} dzResult;

/* ------------------------------------------------------------------------> */

/* An enumeration that represents the type of a NAND flash cell. */
typedef enum dzCellType_ {
    DZ_CELL_TYPE_UNKNOWN = -1,
    DZ_CELL_TYPE_SLC,  // 2 voltage states
    DZ_CELL_TYPE_MLC,  // 4 voltage states
    DZ_CELL_TYPE_TLC,  // 8 voltage states
    DZ_CELL_TYPE_QLC,  // 16 voltage states
    DZ_CELL_TYPE_COUNT_
} dzCellType;

/* ------------------------------------------------------------------------> */

/* An enumeration that represents the status bits of a NAND flash die. */
typedef enum dzDieStatus_ {
    DZ_DIE_STATUS_FAIL = (1 << 0),
    DZ_DIE_STATUS_FAILC = (1 << 1),
    DZ_DIE_STATUS_ARDY = (1 << 5),
    DZ_DIE_STATUS_RDY = (1 << 6),
    DZ_DIE_STATUS_WP = (1 << 7)
} dzDieStatus;

/* ------------------------------------------------------------------------> */

/* An enumeration that represents the ONFI 1.0 command set. */
typedef enum dzChipCommand_ {
    DZ_CHIP_CMD_BLOCK_ERASE_0 = 0x60,
    DZ_CHIP_CMD_BLOCK_ERASE_1 = 0xD0,
    DZ_CHIP_CMD_CHANGE_READ_COLUMN_0 = 0x05,
    DZ_CHIP_CMD_CHANGE_READ_COLUMN_1 = 0xE0,
    DZ_CHIP_CMD_CHANGE_WRITE_COLUMN = 0x85,
    DZ_CHIP_CMD_GET_FEATURES = 0xEE,
    DZ_CHIP_CMD_PAGE_PROGRAM_0 = 0x80,
    DZ_CHIP_CMD_PAGE_PROGRAM_1 = 0x10,
    DZ_CHIP_CMD_READ_0 = 0x00,
    DZ_CHIP_CMD_READ_1 = 0x30,
    DZ_CHIP_CMD_READ_ID = 0x90,
    DZ_CHIP_CMD_READ_PARAMETER_PAGE = 0xEC,
    DZ_CHIP_CMD_READ_STATUS = 0x70,
    DZ_CHIP_CMD_RESET = 0xFF,
    DZ_CHIP_CMD_SET_FEATURES = 0xEF,
    DZ_CHIP_CMD_UNKNOWN = 0xFE
} dzChipCommand;

/* ------------------------------------------------------------------------> */

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

/* ------------------------------------------------------------------------> */

/* A structure that represents the spare area of a NAND flash page. */
typedef struct dzPageSpare_ dzPageSpare;

/* A structure that represents a NAND flash page. */
typedef dzByteArray dzPage;

/* ------------------------------------------------------------------------> */

/* A structure that represents the configuration of a NAND flash die. */
typedef struct dzDieConfig_ {
    dzID dieId;
    dzF32 badBlockRatio;
    dzCellType cellType;
    dzU16 planeCountPerDie;
    dzU16 blockCountPerPlane;
    dzU16 pageCountPerBlock;
    dzU16 pageSizeInBytes;
    dzBool isVerbose;
} dzDieConfig;

/* A structure that represents a NAND flash die. */
typedef struct dzDie_ dzDie;

/* ------------------------------------------------------------------------> */

/* A structure that represents the configuration of a NAND flash chip. */
typedef struct dzChipConfig_ {
    dzDieConfig dieConfig;
    dzID chipId;
    dzByte dieCount;
    dzBool isVerbose;
} dzChipConfig;

/* A structure that represents a NAND flash chip, also known as a 'target'. */
typedef struct dzChip_ dzChip;

/* Constants ==============================================================> */

/* A constant that represents an invalid identifier. */
extern const dzID DZ_API_INVALID_ID;

/* Public Functions =======================================================> */

/* <----------------------------------------------------------- [src/chip.c] */

/* Initializes `*chip` with the given `config`. */
dzResult dzChipInit(dzChip **chip, dzChipConfig config);

/* Releases the memory allocated for `chip`. */
void dzChipDeinit(dzChip *chip);

/* Reads data from `chip`'s I/O bus. */
void dzChipRead(dzChip *chip, dzByte *data, dzTimestamp ts);

/* Writes `data` to `chip`'s I/O bus. */
void dzChipWrite(dzChip *chip, dzByte data, dzTimestamp ts);

/* Waits until `chip` is ready. */
dzTimestamp dzChipWaitUntilReady(dzChip *chip);

/* ------------------------------------------------------------------------> */

/* Returns the current timestamp of `chip`, in microseconds. */
dzTimestamp dzChipGetCurrentTime(const dzChip *chip);

/* ------------------------------------------------------------------------> */

/* Returns the state of the "Address Latch Enable" control line in `chip`. */
dzByte dzChipGetALE(const dzChip *chip);

/* Returns the state of the "Command Latch Enable" control line in `chip`. */
dzByte dzChipGetCLE(const dzChip *chip);

/* Returns the state of the "Chip Enable" control line in `chip`. */
dzByte dzChipGetCE(const dzChip *chip);

/* Returns the state of the "Ready/Busy" control line in `chip`. */
dzByte dzChipGetRB(const dzChip *chip);

/* ------------------------------------------------------------------------> */

/* Sets the state of the "Address Latch Enable" control line in `die`. */
void dzChipSetALE(dzChip *chip, dzByte state);

/* Sets the state of the "Command Latch Enable" control line in `chip`. */
void dzChipSetCLE(dzChip *chip, dzByte state);

/* Sets the state of the "Chip Enable" control line in `chip`. */
void dzChipSetCE(dzChip *chip, dzByte state);

/* Sets the state of the "Write Protect" control line in `chip`. */
void dzChipSetWP(dzChip *chip, dzByte state);

/* ------------------------------------------------------------------------> */

/* Toggles the state of the "Read Enable" control line in `chip`. */
void dzChipToggleRE(dzChip *chip);

/* Toggles the state of the "Write Enable" control line in `chip`. */
void dzChipToggleWE(dzChip *chip);

/* <------------------------------------------------------------ [src/die.c] */

/* Initializes `*die` with the given `config`. */
dzResult dzDieInit(dzDie **die, dzDieConfig config);

/* Releases the memory allocated for `die`. */
void dzDieDeinit(dzDie *die);

/* Performs `command` on `die`. */
void dzDieDecodeCommand(dzDie *die, dzByte command, dzTimestamp ts);

/* Waits until the `die`'s "RDY" status bit is set. */
dzTimestamp dzDieWaitUntilReady(dzDie *die);

/* ------------------------------------------------------------------------> */

/* Returns the "RDY" status bit of `die`. */
dzByte dzDieGetRDY(const dzDie *die);

/* <----------------------------------------------------------- [src/page.c] */

/* Initializes the given `page`. */
dzResult dzPageInit(dzPage page);

/* Returns the size of a NAND flash page's spare area. */
dzUSize dzPageGetSpareAreaSize(void);

/* Returns `true` if `page` is defective. */
dzBool dzPageIsDefective(dzPage page);

/* Marks `page` as defective. */
dzResult dzPageMarkAsDefective(dzPage page);

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

/* Returns the number of leading zero-bits in `x`. */
DZ_API_INLINE dzByte dzUtilsClz(dzU32 x) {
    /* clang-format off */

    if (x == 0U) return UCHAR_MAX;

#if defined(_MSC_VER)
    #if defined(__ARM_ARCH)
        return _CountLeadingZeros(x);
    #elif defined(__BMI__)
        return _lzcnt_u32(x);
    #else
        dzU32 result;

        return _BitScanReverse(&result, x) 
            ? (dzByte) (31U - result)
            : UINT32_MAX;
    #endif
#elif defined(__GNUC__)
    return (dzByte) __builtin_clz(x);
#else
    dzU32 result = 0U;

    if (!(x & 0xFFFF0000U)) result += 16U, x <<= 16U;
    if (!(x & 0xFF000000U)) result += 8U, x <<= 8U;
    if (!(x & 0xF0000000U)) result += 4U, x <<= 4U;
    if (!(x & 0xC0000000U)) result += 2U, x <<= 2U;
    if (!(x & 0x80000000U)) result += 1U;

    return result;
#endif

    /* clang-format on */
}

/* Returns the minimum number of bits required to represent `x`. */
DZ_API_INLINE dzByte dzUtilsGetBitCount(dzU32 x) {
    return (dzByte) (32U - dzUtilsClz(x));
}

/* Returns the number of bytes required to represent a column address. */
DZ_API_INLINE dzByte dzUtilsGetColumnAddressSize(dzU16 pageSizeInBytes) {
    dzByte bitCount = dzUtilsGetBitCount(pageSizeInBytes);

    return (dzByte) ((bitCount + (dzByte) 7U) >> 3U);
}

/* Returns the number of bytes required to represent a row address. */
DZ_API_INLINE dzByte dzUtilsGetRowAddressSize(dzByte dieCount,
                                              dzU32 blockCountPerDie,
                                              dzU16 pageCountPerBlock) {
    dzByte bitCount = (dzByte) (dzUtilsGetBitCount(dieCount)
                                + dzUtilsGetBitCount(blockCountPerDie)
                                + dzUtilsGetBitCount(pageCountPerBlock));

    return (dzByte) ((bitCount + (dzByte) 7U) >> 3U);
}

/* ========================================================================> */

#ifdef __cplusplus
}
#endif  // `__cplusplus`

#endif  // `SSDEEZ_H`
