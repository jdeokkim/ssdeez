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

/* Macro-defined Constants ================================================> */

/* 
    Maximum penalty factor applied to the number of P/E cycles per page,
    for each layer in a block.
*/
#define DZ_PAGE_PE_CYCLE_COUNT_MAX_PENALTY   0.2f

/* 
    Standard deviation for the Gaussian distribution 
    of the maximum P/E cycles per page.
*/

#define DZ_PAGE_PE_CYCLE_COUNT_STDDEV        128.0f

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

/* A structure that represents the configuration of a `dzDie`. */
typedef struct dzDieConfig_ {
    dzCellType cellType;
    dzU32 planeCountPerDie;
    dzU32 blockCountPerPlane;
    dzU32 layerCountPerBlock;
    dzU32 pageSizeInBytes;
} dzDieConfig;

/* ========================================================================> */

/* A structure that represents the metadata of a NAND flash page. */
typedef struct dzPageMetadata_ dzPageMetadata;

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

/* Writes `srcBuffer` to the `pageIndex`-th page in `die`. */
// bool dzDieProgramPage(dzDie *die, dzU64 pageIndex, const void *srcBuffer);

/* 
    Reads data from the `pageIndex`-th page in `die`, 
    then copies it to `dstBuffer`. 
*/
// bool dzDieReadPage(dzDie *die, dzU64 pageIndex, void *dstBuffer);

/* <------------------------------------------------------------ [src/page.c] */

/* Creates an array of page metadata based on the given `config`. */
dzPageMetadata *dzPageCreateMetadata(dzDieConfig config);

/* Releases the memory allocated for `pageMetadata`. */
void dzPageReleaseMetadata(dzPageMetadata *pageMetadata);

/* Marks the `pageIndex`-th page as valid. */
bool dzPageMarkAsValid(dzPageMetadata *pageMetadata, dzU64 pageIndex);

/* Marks the `pageIndex`-th page as free. */
bool dzPageMarkAsFree(dzPageMetadata *pageMetadata, dzU64 pageIndex);

/* <---------------------------------------------------------- [src/utils.c] */

/* Returns a pseudo-random number from a Gaussian distribution. */
dzF32 dzUtilsGaussian(dzF32 mu, dzF32 sigma);

/* ========================================================================> */

#ifdef __cplusplus
}
#endif  // `__cplusplus`

#endif  // `SSDEEZ_H`
