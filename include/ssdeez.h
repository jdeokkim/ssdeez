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

/* Compiler-specific attribute for a function that must be always inlined. */
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

    #define DZ_API_PRIVATE_INLINE static DZ_API_INLINE
#endif  // `DZ_API_INLINE`

/* Ignores the "unused parameter" and "unused variable" errors. */
#define DZ_API_UNUSED_VARIABLE(x) ((void) (x))

/* Macro-defined Constants ================================================> */

// clang-format off

/* Specifies the standard deviation ratio for the erase latency. */
#define DZ_BLOCK_ERASE_LATENCY_STDDEV_RATIO    0.05

/*
    Specifies how much space the OOB (Out-Of-Band) area takes up,
    in relation to the total page size.
*/
#define DZ_PAGE_OUT_OF_BAND_SIZE_RATIO         0.05

/* 
    Maximum penalty factor applied to the maximum number of 
    P/E cycles per page, for each layer in a block.
*/
#define DZ_PAGE_PE_CYCLE_COUNT_MAX_PENALTY     0.25

/*
    Specifies the standard deviation ratio for initializing 
    the maximum number of P/E cycles per page.
*/
#define DZ_PAGE_PE_CYCLE_COUNT_STDDEV_RATIO    0.1

/* Specifies the standard deviation ratio for the program latency. */
#define DZ_PAGE_PROGRAM_LATENCY_STDDEV_RATIO   0.1

/* Specifies the standard deviation ratio for the read latency. */
#define DZ_PAGE_READ_LATENCY_STDDEV_RATIO      0.025

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

// clang-format on

/* ========================================================================> */

/* An enumeration that represents the state of a NAND flash block. */
typedef enum dzBlockState_ {
    DZ_BLOCK_STATE_UNKNOWN = -1,
    DZ_BLOCK_STATE_FREE,
    DZ_BLOCK_STATE_VICTIM,
    DZ_BLOCK_STATE_ACTIVE,
    DZ_BLOCK_STATE_BAD,
    DZ_BLOCK_STATE_RESERVED,
    DZ_BLOCK_STATE_COUNT_
} dzBlockState;

/* An enumeration that represents the state of a NAND flash page. */
typedef enum dzPageState_ {
    DZ_PAGE_STATE_UNKNOWN = -1,
    DZ_PAGE_STATE_FREE,
    DZ_PAGE_STATE_INVALID,
    DZ_PAGE_STATE_VALID,
    DZ_PAGE_STATE_BAD,
    DZ_PAGE_STATE_RESERVED,
    DZ_PAGE_STATE_COUNT_
} dzPageState;

/* An enumeration that represents the type of a NAND flash cell. */
typedef enum dzCellType_ {
    DZ_CELL_TYPE_UNKNOWN = -1,
    DZ_CELL_TYPE_SLC,  // 2 voltage states
    DZ_CELL_TYPE_MLC,  // 4 voltage states
    DZ_CELL_TYPE_TLC,  // 8 voltage states
    DZ_CELL_TYPE_QLC,  // 16 voltage states
    DZ_CELL_TYPE_COUNT_
} dzCellType;

/* ========================================================================> */

/* A structure that represents a physical page address. */
typedef struct dzPPA_ {
    // dzU64 channelId;
    dzU64 chipId;
    dzU64 dieId;
    dzU64 planeId;
    dzU64 blockId;
    dzU64 pageId;
} dzPPA;

/* A structure that represents a physical block address. */
typedef dzPPA dzPBA;

/* ========================================================================> */

/* A structure that represents a group of NAND flash planes. */
typedef struct dzDie_ dzDie;

/* A structure that represents the configuration of a NAND flash die. */
typedef struct dzDieConfig_ {
    dzU64 dieId;
    dzCellType cellType;
    dzF64 badBlockRatio;
    dzU32 planeCountPerDie;
    dzU32 blockCountPerPlane;
    dzU32 pageCountPerBlock;
    dzU32 pageSizeInBytes;
} dzDieConfig;

/* A structure that represents the metadata of a NAND flash die. */
typedef struct dzDieMetadata_ dzDieMetadata;

/* A structure that represents various statistics of a NAND flash die. */
typedef struct dzDieStatistics_ dzDieStatistics;

/* ========================================================================> */

/* A structure that represents the configuration of a NAND flash plane. */
typedef struct dzPlaneConfig_ {
    dzU64 planeId;
    dzU64 blockCount;
    // TODO: ...
} dzPlaneConfig;

/* A structure that represents the metadata of a NAND flash plane. */
typedef struct dzPlaneMetadata_ dzPlaneMetadata;

/* ========================================================================> */

/* A structure that represents the configuration of a NAND flash block. */
typedef struct dzBlockConfig_ {
    dzPBA pba;
    dzU64 pageCount;
    dzCellType cellType;
    // TODO: ...
} dzBlockConfig;

/* A structure that represents the metadata of a NAND flash block. */
typedef struct dzBlockMetadata_ dzBlockMetadata;

/* ========================================================================> */

/* A structure that represents the configuration of a NAND flash page. */
typedef struct dzPageConfig_ {
    dzPPA ppa;
    dzF64 peCycleCountPenalty;
    dzU32 pageSizeInBytes;
    dzCellType cellType;
} dzPageConfig;

/* A structure that represents the metadata of a NAND flash page. */
typedef struct dzPageMetadata_ dzPageMetadata;

/* ========================================================================> */

/* A structure that represents a buffer with a fixed size. */
typedef struct dzSizedBuffer_ {
    dzByte *ptr;
    dzUSize size;
} dzSizedBuffer;

/* Constants ==============================================================> */

/* A constant that represents an invalid chip identifier. */
extern const dzU64 DZ_CHIP_INVALID_ID;

/* A constant that represents an invalid die identifier. */
extern const dzU64 DZ_DIE_INVALID_ID;

/* A constant that represents an invalid plane identifier. */
extern const dzU64 DZ_PLANE_INVALID_ID;

/* A constant that represents an invalid block identifier. */
extern const dzU64 DZ_BLOCK_INVALID_ID;

/* A constant that represents an invalid page identifier. */
extern const dzU64 DZ_PAGE_INVALID_ID;

/* Public Functions =======================================================> */

/* <---------------------------------------------------------- [src/block.c] */

/* Initializes a block `metadata` within the given region. */
bool dzBlockInitMetadata(dzBlockMetadata *metadata, dzBlockConfig config);

/* De-initializes the block `metadata`. */
void dzBlockDeinitMetadata(dzBlockMetadata *metadata);

/* Returns the size of `dzBlockMetadata`. */
dzUSize dzBlockGetMetadataSize(void);

/* Writes the next page identifier of a block to `nextPageId`. */
bool dzBlockGetNextPageId(dzBlockMetadata *metadata, dzU64 *nextPageId);

/* Returns the current state of a block. */
dzBlockState dzBlockGetState(const dzBlockMetadata *metadata);

/* Returns the number of valid pages in a block. */
dzU64 dzBlockGetValidPageCount(const dzBlockMetadata *metadata);

/* Advances the next page identifier of a block. */
bool dzBlockAdvanceNextPageId(dzBlockMetadata *metadata);

/* Marks a block as active. */
bool dzBlockMarkAsActive(dzBlockMetadata *metadata);

/* Marks a block as bad. */
bool dzBlockMarkAsBad(dzBlockMetadata *metadata);

/* Marks a block as free. */
bool dzBlockMarkAsFree(dzBlockMetadata *metadata, dzF64 *eraseLatency);

/* Marks a block as unknown. */
bool dzBlockMarkAsUnknown(dzBlockMetadata *metadata);

/* Updates the state of the given page within a block's page state map. */
bool dzBlockUpdatePageStateMap(dzBlockMetadata *metadata,
                               dzPPA ppa,
                               dzPageState pageState);

/* <------------------------------------------------------------ [src/die.c] */

/* Creates a die with the given `config`. */
dzDie *dzDieCreate(dzDieConfig config);

/* Releases the memory allocated for the `die`. */
void dzDieRelease(dzDie *die);

/* Returns the total number of blocks in `die`. */
dzU64 dzDieGetBlockCount(const dzDie *die);

/* Returns the current state of the block corresponding to `pba` in `die`. */
dzBlockState dzDieGetBlockState(const dzDie *die, dzPBA pba);

/* Returns the first physical block address within `die`. */
dzPBA dzDieGetFirstPBA(const dzDie *die);

/* Returns the first physical page address within `die`. */
dzPPA dzDieGetFirstPPA(const dzDie *die);

/* Returns the next physical block address following `ppa` within `die`. */
dzPBA dzDieGetNextPBA(const dzDie *die, dzPBA pba);

/* Returns the next physical page address following `ppa` within `die`. */
dzPPA dzDieGetNextPPA(const dzDie *die, dzPPA ppa);

/* Returns the total number of pages in `die`. */
dzU64 dzDieGetPageCount(const dzDie *die);

/* 
    Returns the current state of the page 
    corresponding to `ppa` within `die`. 
*/
dzPageState dzDieGetPageState(const dzDie *die, dzPPA ppa);

/* Writes `srcBuffer` to the page corresponding to `ppa` in `die`. */
bool dzDieProgramPage(dzDie *die, dzPPA ppa, dzSizedBuffer srcBuffer);

/* 
    Reads data from the page corresponding to `ppa` in `die`, 
    copying it to `dstBuffer`. 
*/
bool dzDieReadPage(dzDie *die, dzPPA ppa, dzSizedBuffer dstBuffer);

/* Erases the block corresponding to `pba` in `die`. */
bool dzDieEraseBlock(dzDie *die, dzPBA pba);

/* <----------------------------------------------------------- [src/page.c] */

/* Initializes a page metadata within the given `pagePtr`. */
bool dzPageInitMetadata(dzByte *pagePtr, dzPageConfig config);

/* Returns the size of `dzPageMetadata`. */
dzUSize dzPageGetMetadataSize(void);

/* Returns the physical page address of a page. */
dzPPA dzPageGetPPA(const dzByte *pagePtr, dzU32 pageSizeInBytes);

/* Returns the current state of a page. */
dzPageState dzPageGetState(const dzByte *pagePtr, dzU32 pageSizeInBytes);

/* Returns the read latency of a page. */
bool dzPageGetReadLatency(const dzByte *pagePtr,
                          dzU32 pageSizeInBytes,
                          dzF64 *readLatency);

/* Marks a page as bad. */
bool dzPageMarkAsBad(dzByte *pagePtr, dzU32 pageSizeInBytes);

/* Marks a page as free. */
bool dzPageMarkAsFree(dzByte *pagePtr, dzU32 pageSizeInBytes);

/* Marks a page as unknown. */
bool dzPageMarkAsUnknown(dzByte *pagePtr, dzU32 pageSizeInBytes);

/* Marks a page as valid. */
bool dzPageMarkAsValid(dzByte *pagePtr,
                       dzU32 pageSizeInBytes,
                       dzF64 *programLatency);

/* <---------------------------------------------------------- [src/plane.c] */

/* Initializes a plane metadata within the given `metadata` region. */
bool dzPlaneInitMetadata(dzPlaneMetadata *metadata, dzPlaneConfig config);

/* De-initializes the plane `metadata`. */
void dzPlaneDeinitMetadata(dzPlaneMetadata *metadata);

/* Returns the size of `dzPlaneMetadata`. */
dzUSize dzPlaneGetMetadataSize(void);

/* Updates the state of the given block within a plane's block state map. */
bool dzPlaneUpdateBlockStateMap(dzPlaneMetadata *metadata,
                                dzPBA pba,
                                dzBlockState blockState);

/* <---------------------------------------------------------- [src/utils.c] */

/* Returns a pseudo-random number from a Gaussian distribution. */
dzF64 dzUtilsGaussian(dzF64 mu, dzF64 sigma);

/* Returns a pseudo-random unsigned 64-bit integer. */
dzU64 dzUtilsRand(void);

/* 
    Returns a pseudo-random unsigned 64-bit integer 
    in the given (inclusive) range. 
*/
dzU64 dzUtilsRandRange(dzU64 min, dzU64 max);

/* 
    Returns a pseudo-random double-precision floating-point number 
    in the given (exclusive) range. 
*/
dzF64 dzUtilsRandRangeF64(dzF64 min, dzF64 max);

/* ========================================================================> */

/* Returns `true` if `pba1` equals to `pba2`. */
DZ_API_INLINE bool dzUtilsPbaEquals(dzPBA pba1, dzPBA pba2) {
    return (pba1.chipId == pba2.chipId) && (pba1.dieId == pba2.dieId)
           && (pba1.planeId == pba2.planeId) && (pba1.blockId == pba2.blockId);
}

/* ========================================================================> */

#ifdef __cplusplus
}
#endif  // `__cplusplus`

#endif  // `SSDEEZ_H`
