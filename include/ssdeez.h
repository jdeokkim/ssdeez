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
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
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

/* Macro-defined Constants ================================================> */

/*
    Specifies how much space the OOB (Out-Of-Band) area takes up,
    in relation to the total page size.
*/
#define DZ_PAGE_OUT_OF_BAND_SIZE_RATIO       0.05

/* 
    Maximum penalty factor applied to the maximum number of 
    P/E cycles per page, for each layer in a block.
*/
#define DZ_PAGE_PE_CYCLE_COUNT_MAX_PENALTY   0.25

/*
    Specifies the standard deviation ratio for initializing 
    the maximum number of P/E cycles per page.
*/
#define DZ_PAGE_PE_CYCLE_COUNT_STDDEV_RATIO  0.1

/* Typedefs ===============================================================> */

/* Aliases for primitive integer types. */

typedef bool          dzBool;

typedef unsigned char dzByte;

typedef ptrdiff_t     dzISize;
typedef size_t        dzUSize;

typedef int32_t       dzI32;
typedef int64_t       dzI64;

typedef uint32_t      dzU32;
typedef uint64_t      dzU64;

typedef float         dzF32;
typedef double        dzF64;

/* ========================================================================> */

/* An enumeration that represents the type of a NAND flash cell. */
typedef enum dzCellType_ {
    DZ_CELL_TYPE_UNKNOWN = -1,
    DZ_CELL_TYPE_SLC,          // 1 voltage state
    DZ_CELL_TYPE_MLC,          // 4 voltage states
    DZ_CELL_TYPE_TLC,          // 8 voltage states
    DZ_CELL_TYPE_QLC,          // 16 voltage states
    DZ_CELL_TYPE_COUNT_
} dzCellType;

/* An enumeration that represents the state of a NAND flash page. */
typedef enum dzPageState_ {
    DZ_PAGE_STATE_UNKNOWN = -1,
    DZ_PAGE_STATE_FREE,
    DZ_PAGE_STATE_VALID,
    DZ_PAGE_STATE_INVALID,
    DZ_PAGE_STATE_CORRUPTED,
    DZ_PAGE_STATE_RESERVED,
    DZ_PAGE_STATE_COUNT_
} dzPageState;

/* ========================================================================> */

/* 
    A structure that represents an interface between the controller 
    and a group of dies. 
*/
typedef struct dzChannel_ dzChannel;

/* ========================================================================> */

/* A structure that represents a group of NAND flash planes. */
typedef struct dzDie_ dzDie;

/* A structure that represents the configuration of a NAND flash die. */
typedef struct dzDieConfig_ {
    dzCellType cellType;
    dzU32 planeCountPerDie;
    dzU32 blockCountPerPlane;
    dzU32 layerCountPerBlock;
    dzU32 pageSizeInBytes;
} dzDieConfig;

/* A structure that represents the metadata of a NAND flash die. */
typedef struct dzDieMetadata_ dzDieMetadata;

/* ========================================================================> */

/* A structure that represents the configuration of a NAND flash page. */
typedef struct dzPageConfig_ {
    dzCellType cellType;
    dzU32 pageSizeInBytes;
    dzF64 peCycleCountPenalty;
} dzPageConfig;

/* A structure that represents the metadata of a NAND flash page. */
typedef struct dzPageMetadata_ dzPageMetadata;

/* Constants ==============================================================> */

/* A constant that represents an invalid physical page number. */
extern const dzU64 DZ_PAGE_INVALID_PPN;

/* Public Functions =======================================================> */

/* <-------------------------------------------------------- [src/channel.c] */

/* Creates a channel with the given `id`. */
dzChannel *dzChannelCreate(dzU64 id);

/* Releases the memory allocated for the `channel`. */
void dzChannelRelease(dzChannel *channel);

/* <------------------------------------------------------------ [src/die.c] */

/* Creates a die with the given `config`. */
dzDie *dzDieCreate(dzDieConfig config);

/* Releases the memory allocated for the `die`. */
void dzDieRelease(dzDie *die);

/* Converts `pagePtr` to a physical page number. */
dzU64 dzDiePtrToPPN(const dzDie *die, const dzByte *pagePtr);

/* Converts `ppn` to a physical page address. */
dzByte *dzDiePPNToPtr(const dzDie *die, dzU64 ppn);

/* <------------------------------------------------------------ [src/page.c] */

/* Initializes a page metadata object within the given `pageBuffer`. */
bool dzPageInitMetadata(dzByte *pageBuffer, dzPageConfig config);

/* Returns the size of `dzPageMetadata`. */
dzUSize dzPageGetMetadataSize(void);

/* Marks a page as valid. */
bool dzPageMarkAsValid(dzByte *pageBuffer,
                       dzU32 pageSizeInBytes,
                       dzF64 *outLatency);

/* Marks a page as free. */
bool dzPageMarkAsFree(dzByte *pageBuffer,
                      dzU32 pageSizeInBytes,
                      dzF64 *outLatency);

/* <---------------------------------------------------------- [src/utils.c] */

/* Returns a pseudo-random number from a Gaussian distribution. */
dzF64 dzUtilsGaussian(dzF64 mu, dzF64 sigma);

/* Inline Functions =======================================================> */

/* Returns `true` if `cellType` is a valid NAND flash cell type. */
DZ_API_INLINE bool dzIsValidCellType(dzCellType cellType) {
    return (cellType > DZ_CELL_TYPE_UNKNOWN && cellType < DZ_CELL_TYPE_COUNT_);
}

/* ========================================================================> */

#ifdef __cplusplus
}
#endif  // `__cplusplus`

#endif  // `SSDEEZ_H`
