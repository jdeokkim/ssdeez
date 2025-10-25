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

#include <float.h>
#include <math.h>
#include <string.h>

#include "ssdeez.h"

/* Macros =================================================================> */

#define dzDieForEachPage(dieBuffer, dieMetadata, ptrIdentifier) \
    for (dzByte *ptrIdentifier = (dieBuffer),                   \
                *ptrIdentifier##__LINE__ =                      \
                    (dieBuffer)                                 \
                    + ((dieMetadata).pageCountPerDie            \
                       * (dieMetadata).physicalPageSize);       \
         ptrIdentifier < ptrIdentifier##__LINE__;               \
         ptrIdentifier += (dieMetadata).physicalPageSize)

#define dzDieForEachPageInBlock(dieBuffer,                         \
                                dieMetadata,                       \
                                blockIndex,                        \
                                ptrIdentifier)                     \
    for (dzByte *ptrIdentifier =                                   \
             (dieBuffer)                                           \
             + ((blockIndex) * ((dieMetadata).physicalBlockSize)), \
                *ptrIdentifier##__LINE__ =                         \
                    (dieBuffer)                                    \
                    + (((blockIndex) + 1U)                         \
                       * ((dieMetadata).physicalBlockSize));       \
         ptrIdentifier < ptrIdentifier##__LINE__;                  \
         ptrIdentifier += (dieMetadata).physicalPageSize)

/* Typedefs ===============================================================> */

/* A structure that represents the metadata of a NAND flash die. */
struct dzDieMetadata_ {
    dzPlaneMetadata *planes;
    dzBlockMetadata *blocks;
    dzU64 blockCountPerDie;
    dzU64 pageCountPerDie;
    dzU64 pageCountPerPlane;
    dzU64 physicalBlockSize;
    dzU64 physicalPageSize;
};

/* A structure that represents various statistics of a NAND flash die. */
struct dzDieStatistics_ {
    dzF64 totalProgramLatency;
    dzU64 totalProgramCount;
    dzF64 totalReadLatency;
    dzU64 totalReadCount;
    dzF64 totalEraseLatency;
    dzU64 totalEraseCount;
    // TODO: ...
};

/* A structure that represents a group of NAND flash planes. */
struct dzDie_ {
    dzDieConfig config;
    dzDieStatistics stats;
    dzDieMetadata metadata;
    dzByte *buffer;
    // TODO: ...
};

/* Constants ==============================================================> */

/* A constant that represents an invalid die identifier. */
const dzU64 DZ_DIE_INVALID_ID = UINT64_MAX;

/* Private Variables ======================================================> */

// TODO: ...

/* Private Function Prototypes ============================================> */

/* Mark a random number of blocks as bad. */
static bool dzDieCorruptRandomBlocks(dzDie *die);

/* Creates a die buffer with the given `config` and `metadata`. */
static dzByte *dzDieCreateBuffer(dzDieConfig config, dzDieMetadata metadata);

/* 
    Returns the previous or the next index of 
    the given `blockIndex` in `die`. 
*/
static dzU64 dzDieGetAdjacentBlockIndex(dzDie *die, dzU64 blockIndex);

/* Initializes all block metadata in `die.` */
static bool dzDieInitBlockMetadata(dzDie *die);

/* Initializes all plane metadata in `die.` */
static bool dzDieInitPlaneMetadata(dzDie *die);

/* Initializes a `die` metadata. */
static bool dzDieInitMetadata(dzDie *die);

/* ========================================================================> */

/* 
    Returns `true` if `pba` is pointing to the 
    valid physical block address within `die`.
*/
DZ_API_PRIVATE_INLINE dzBool dzDieIsValidPBA(const dzDie *die, dzPBA pba);

/* 
    Returns `true` if `ppa` is pointing to the 
    valid physical page address within `die`.
*/
DZ_API_PRIVATE_INLINE dzBool dzDieIsValidPPA(const dzDie *die, dzPPA ppa);

/* Returns the pointer to a page corresponding to `ppa` in `die`. */
DZ_API_PRIVATE_INLINE dzByte *dzDiePPAToPtr(const dzDie *die, dzPPA ppa);

/* Public Functions =======================================================> */

/* Creates a die with the given `config`. */
dzDie *dzDieCreate(dzDieConfig config) {
    // clang-format off

    if (config.dieId == DZ_DIE_INVALID_ID
        || config.cellType <= DZ_CELL_TYPE_UNKNOWN
        || config.cellType >= DZ_CELL_TYPE_COUNT_
        || config.planeCountPerDie == 0U 
        || config.blockCountPerPlane == 0U
        || config.pageCountPerBlock == 0U 
        || config.pageSizeInBytes == 0U)
        return NULL;

    // clang-format on

    dzDie *die = malloc(sizeof *die);

    if (die == NULL) return die;

    {
        die->config = config;

        die->stats = (dzDieStatistics) { .totalProgramLatency = 0.0,
                                         .totalProgramCount = 0U,
                                         .totalReadLatency = 0.0,
                                         .totalReadCount = 0U };

        die->metadata = (dzDieMetadata) { .planes = NULL, .blocks = NULL };
    }

    if (!dzDieInitMetadata(die)
        || (die->buffer = dzDieCreateBuffer(config, die->metadata)) == NULL
        || !dzDieCorruptRandomBlocks(die)) {
        dzDieRelease(die);

        return NULL;
    }

    return die;
}

/* Releases the memory allocated for `die`. */
void dzDieRelease(dzDie *die) {
    if (die == NULL) return;

    for (dzU64 i = 0U; i < die->metadata.blockCountPerDie; i++) {
        dzBlockMetadata *blockMetadata =
            (dzBlockMetadata *) (((dzByte *) die->metadata.blocks)
                                 + (i * dzBlockGetMetadataSize()));

        dzBlockDeinitMetadata(blockMetadata);
    }

    for (dzU64 i = 0U; i < die->config.planeCountPerDie; i++) {
        dzPlaneMetadata *planeMetadata =
            (dzPlaneMetadata *) (((dzByte *) die->metadata.planes)
                                 + (i * dzPlaneGetMetadataSize()));

        dzPlaneDeinitMetadata(planeMetadata);
    }

    free(die->metadata.planes), free(die->buffer), free(die);
}

/* Returns the total number of blocks in `die`. */
dzU64 dzDieGetBlockCount(const dzDie *die) {
    return (die != NULL) ? die->metadata.blockCountPerDie : 0U;
}

/* Returns the current state of the block corresponding to `pba` in `die`. */
dzBlockState dzDieGetBlockState(const dzDie *die, dzPBA pba) {
    if (!dzDieIsValidPBA(die, pba)) return DZ_BLOCK_STATE_UNKNOWN;

    dzU64 blockIndex = (pba.planeId * die->config.blockCountPerPlane)
                       + pba.blockId;

    const dzBlockMetadata *blockMetadata =
        (const dzBlockMetadata *) (((dzByte *) die->metadata.blocks)
                                   + (blockIndex * dzBlockGetMetadataSize()));

    return dzBlockGetState(blockMetadata);
}

/* Returns the first physical block address within `die`. */
dzPBA dzDieGetFirstPBA(const dzDie *die) {
    return (dzPBA) { .chipId = DZ_CHIP_INVALID_ID,
                     .dieId = ((die != NULL) ? die->config.dieId
                                             : DZ_DIE_INVALID_ID),
                     .planeId = ((die != NULL) ? 0U : DZ_PLANE_INVALID_ID),
                     .blockId = ((die != NULL) ? 0U : DZ_BLOCK_INVALID_ID) };
}

/* Returns the first physical page address within `die`. */
dzPPA dzDieGetFirstPPA(const dzDie *die) {
    return (dzPPA) { .chipId = DZ_CHIP_INVALID_ID,
                     .dieId = ((die != NULL) ? die->config.dieId
                                             : DZ_DIE_INVALID_ID),
                     .planeId = ((die != NULL) ? 0U : DZ_PLANE_INVALID_ID),
                     .blockId = ((die != NULL) ? 0U : DZ_BLOCK_INVALID_ID),
                     .pageId = ((die != NULL) ? 0U : DZ_PAGE_INVALID_ID) };
}

/* Returns the next physical block address following `ppa` within `die`. */
dzPBA dzDieGetNextPBA(const dzDie *die, dzPBA pba) {
    // clang-format off

    if (!dzDieIsValidPBA(die, pba))
        return (dzPBA) {
            .chipId = DZ_CHIP_INVALID_ID, 
            .dieId = DZ_DIE_INVALID_ID,
            .planeId = DZ_PLANE_INVALID_ID, 
            .blockId = DZ_BLOCK_INVALID_ID,
            .pageId = DZ_PAGE_INVALID_ID
        };

    // clang-format on

    pba.pageId = 0U, pba.blockId++;

    if (pba.blockId >= die->config.blockCountPerPlane)
        pba.planeId++, pba.blockId = 0U;

    return (pba.planeId < die->config.planeCountPerDie)
               ? pba
               : ((dzPBA) { .chipId = DZ_CHIP_INVALID_ID,
                            .dieId = DZ_DIE_INVALID_ID,
                            .planeId = DZ_PLANE_INVALID_ID,
                            .blockId = DZ_BLOCK_INVALID_ID,
                            .pageId = DZ_PAGE_INVALID_ID });
}

/* Returns the next physical page address following `ppa` within `die`. */
dzPPA dzDieGetNextPPA(const dzDie *die, dzPPA ppa) {
    // clang-format off

    if (!dzDieIsValidPPA(die, ppa))
        return (dzPPA) {
            .chipId = DZ_CHIP_INVALID_ID, 
            .dieId = DZ_DIE_INVALID_ID,
            .planeId = DZ_PLANE_INVALID_ID, 
            .blockId = DZ_BLOCK_INVALID_ID,
            .pageId = DZ_PAGE_INVALID_ID
        };

    // clang-format on

    ppa.pageId++;

    if (ppa.pageId >= die->config.pageCountPerBlock)
        ppa.blockId++, ppa.pageId = 0U;

    if (ppa.blockId >= die->config.blockCountPerPlane)
        ppa.planeId++, ppa.blockId = 0U;

    return (ppa.planeId < die->config.planeCountPerDie)
               ? ppa
               : ((dzPPA) { .chipId = DZ_CHIP_INVALID_ID,
                            .dieId = DZ_DIE_INVALID_ID,
                            .planeId = DZ_PLANE_INVALID_ID,
                            .blockId = DZ_BLOCK_INVALID_ID,
                            .pageId = DZ_PAGE_INVALID_ID });
}

/* Returns the total number of pages in `die`. */
dzU64 dzDieGetPageCount(const dzDie *die) {
    return (die != NULL) ? die->metadata.pageCountPerDie : 0U;
}

/* 
    Returns the current state of the page 
    corresponding to `ppa` within `die`. 
*/
dzPageState dzDieGetPageState(const dzDie *die, dzPPA ppa) {
    const dzByte *pagePtr = (const dzByte *) dzDiePPAToPtr(die, ppa);

    return dzPageGetState(pagePtr, die->config.pageSizeInBytes);
}

/* Writes `srcBuffer` to the page corresponding to `ppa` in `die`. */
bool dzDieProgramPage(dzDie *die, dzPPA ppa, dzSizedBuffer srcBuffer) {
    dzByte *pagePtr = dzDiePPAToPtr(die, ppa);

    if (pagePtr == NULL || srcBuffer.ptr == NULL || srcBuffer.size == 0U)
        return false;

    dzU64 blockIndex = (ppa.planeId * die->config.blockCountPerPlane)
                       + ppa.blockId;

    dzBlockMetadata *blockMetadata =
        (dzBlockMetadata *) (((dzByte *) die->metadata.blocks)
                             + (blockIndex * dzBlockGetMetadataSize()));

    {
        dzU64 nextPageId = DZ_PAGE_INVALID_ID;

        // NOTE: Enforce "Sequential Page Programming"
        if (!dzBlockGetNextPageId(blockMetadata, &nextPageId)
            || ppa.pageId != nextPageId)
            return false;
    }

    {
        dzF64 programLatency = -DBL_MAX;

        // NOTE: Erase-before-Write Property!
        if (!dzPageMarkAsValid(pagePtr,
                               die->config.pageSizeInBytes,
                               &programLatency)
            || (programLatency < 0.0))
            return false;

        die->stats.totalProgramLatency += programLatency;
        die->stats.totalProgramCount++;
    }

    {
        dzPageState pageState = dzDieGetPageState(die, ppa);

        if (!dzBlockUpdatePageStateMap(blockMetadata, ppa, pageState))
            return false;

        /*
            NOTE: Since at least one valid page is present in this block, 
                  it can be marked as active
        */
        if (!dzBlockAdvanceNextPageId(blockMetadata)
            || !dzBlockMarkAsActive(blockMetadata))
            return false;
    }

    dzPlaneMetadata *planeMetadata =
        (dzPlaneMetadata *) (((dzByte *) die->metadata.planes)
                             + (ppa.planeId * dzPlaneGetMetadataSize()));

    {
        dzBlockState blockState = dzDieGetBlockState(die, ppa);

        if (!dzPlaneUpdateBlockStateMap(planeMetadata, ppa, blockState))
            return false;
    }

    (void) memcpy(pagePtr,
                  srcBuffer.ptr,
                  ((srcBuffer.size < die->config.pageSizeInBytes)
                       ? srcBuffer.size
                       : die->config.pageSizeInBytes));

    return true;
}

/* 
    Reads data from the page corresponding to `ppa` in `die`, 
    copying it to `dstBuffer`. 
*/
bool dzDieReadPage(dzDie *die, dzPPA ppa, dzSizedBuffer dstBuffer) {
    dzByte *pagePtr = dzDiePPAToPtr(die, ppa);

    if (pagePtr == NULL || dstBuffer.ptr == NULL || dstBuffer.size == 0U)
        return false;

    {
        dzF64 readLatency = -DBL_MAX;

        if (!dzPageGetReadLatency(pagePtr,
                                  die->config.pageSizeInBytes,
                                  &readLatency)
            || (readLatency < 0.0))
            return false;

        die->stats.totalReadLatency += readLatency;
        die->stats.totalReadCount++;
    }

    (void) memcpy(dstBuffer.ptr,
                  pagePtr,
                  ((dstBuffer.size < die->config.pageSizeInBytes)
                       ? dstBuffer.size
                       : die->config.pageSizeInBytes));

    return true;
}

/* Erases the block corresponding to `pba` in `die`. */
bool dzDieEraseBlock(dzDie *die, dzPBA pba) {
    if (die == NULL || !dzDieIsValidPBA(die, pba)) return false;

    bool result = true;

    dzU64 blockIndex = (pba.planeId * die->config.blockCountPerPlane)
                       + pba.blockId;

    dzDieForEachPageInBlock(die->buffer, die->metadata, blockIndex, pagePtr) {
        (void) memset(pagePtr, 0xFF, die->config.pageSizeInBytes);

        if (!dzPageMarkAsFree(pagePtr, die->config.pageSizeInBytes)) {
            result = false;

            break;
        }
    }

    dzBlockMetadata *blockMetadata =
        (dzBlockMetadata *) (((dzByte *) die->metadata.blocks)
                             + (blockIndex * dzBlockGetMetadataSize()));

    if (!result) {
        /* NOTE: Mark all pages in the block as bad */

        // clang-format off

        dzDieForEachPageInBlock(die->buffer, die->metadata, blockIndex, pagePtr)
            ((void) dzPageMarkAsBad(pagePtr, die->config.pageSizeInBytes));

        // clang-format on

        (void) dzBlockMarkAsBad(blockMetadata);

        {
            dzPlaneMetadata *planeMetadata =
                (dzPlaneMetadata *) (((dzByte *) die->metadata.planes)
                                     + (pba.planeId
                                        * dzPlaneGetMetadataSize()));

            dzBlockState blockState = dzDieGetBlockState(die, pba);

            if (!dzPlaneUpdateBlockStateMap(planeMetadata, pba, blockState))
                return false;
        }

        return result;
    }

    {
        dzF64 eraseLatency = -DBL_MAX;

        if (!dzBlockMarkAsFree(blockMetadata, &eraseLatency)
            || (eraseLatency < 0.0))
            return false;

        die->stats.totalEraseLatency += eraseLatency;
        die->stats.totalEraseCount++;
    }

    {
        dzPlaneMetadata *planeMetadata =
            (dzPlaneMetadata *) (((dzByte *) die->metadata.planes)
                                 + (pba.planeId * dzPlaneGetMetadataSize()));

        dzBlockState blockState = dzDieGetBlockState(die, pba);

        if (!dzPlaneUpdateBlockStateMap(planeMetadata, pba, blockState))
            return false;
    }

    return result;
}

/* Private Functions ======================================================> */

/* Mark a random number of blocks as bad. */
static bool dzDieCorruptRandomBlocks(dzDie *die) {
    if (die == NULL || die->metadata.blockCountPerDie <= 1) return false;

    dzU64 blockCountPerDie = die->metadata.blockCountPerDie;

    if (die->config.badBlockRatio <= 0.0) return true;

    dzF64 badBlockRatio = dzUtilsRandRangeF64(0.001,
                                              die->config.badBlockRatio);

    dzU64 badBlockCount = (dzU64) ceil(badBlockRatio
                                       * (dzF64) blockCountPerDie);

    if (badBlockCount == 0U) badBlockCount++;

    // NOTE: Block #0 is always guaranteed to be a 'good' block
    dzU64 blockIndex = dzUtilsRandRange(1U, blockCountPerDie - 1);

    while (badBlockCount > 0) {
        dzBlockMetadata *blockMetadata =
            (dzBlockMetadata *) (((dzByte *) die->metadata.blocks)
                                 + (blockIndex * dzBlockGetMetadataSize()));

        if (dzBlockGetState(blockMetadata) == DZ_BLOCK_STATE_BAD) continue;

        // clang-format off

        dzDieForEachPageInBlock(die->buffer, die->metadata, blockIndex, pagePtr) {
            ((void) dzPageMarkAsUnknown(pagePtr, die->config.pageSizeInBytes));
            ((void) dzPageMarkAsBad(pagePtr, die->config.pageSizeInBytes));
        }

        // clang-format on

        (void) dzBlockMarkAsUnknown(blockMetadata);
        (void) dzBlockMarkAsBad(blockMetadata);

        {
            dzPBA pba = dzBlockGetPBA(blockMetadata);

            dzPlaneMetadata *planeMetadata =
                (dzPlaneMetadata *) (((dzByte *) die->metadata.planes)
                                     + (pba.planeId
                                        * dzPlaneGetMetadataSize()));

            dzBlockState blockState = dzDieGetBlockState(die, pba);

            if (!dzPlaneUpdateBlockStateMap(planeMetadata, pba, blockState))
                return false;
        }

        dzU64 newBlockIndex = dzDieGetAdjacentBlockIndex(die, blockIndex);

        // NOTE: Corruption may or may not spread within an one-block radius
        if ((dzUtilsRand() & 1U) || newBlockIndex == DZ_BLOCK_INVALID_ID)
            blockIndex = dzUtilsRandRange(1U, blockCountPerDie - 1);
        else
            blockIndex = newBlockIndex;

        badBlockCount--;
    }

    return true;
}

/* Creates a die buffer with the given `config` and `metadata`. */
static dzByte *dzDieCreateBuffer(dzDieConfig config, dzDieMetadata metadata) {
    dzByte *result = NULL;

    dzU64 bufferSize = metadata.pageCountPerDie * metadata.physicalPageSize;

    if ((result = malloc(bufferSize * sizeof *result)) == NULL) return result;

    // NOTE: Simulating the 'erased' state by setting all bits to `1`
    (void) memset(result, 0xFF, config.pageSizeInBytes);

    dzU64 pageIndex = 0U, centerPageIndex = metadata.pageCountPerDie >> 1U;

    dzDieForEachPage(result, metadata, pagePtr) {
        dzF64 peCycleCountPenalty = 1.0;

        // NOTE: Pages in the top and bottom layers should have lower endurance
        if (metadata.pageCountPerDie >= 2U) {
            dzU64 distanceFromCenter = (pageIndex > centerPageIndex)
                                           ? (pageIndex - centerPageIndex)
                                           : (centerPageIndex - pageIndex);

            dzF64 penaltyScale = ((dzF64) distanceFromCenter
                                  / (dzF64) centerPageIndex);

            peCycleCountPenalty -= (penaltyScale
                                    * DZ_PAGE_PE_CYCLE_COUNT_MAX_PENALTY);
        }

        dzPPA ppa = {
            .dieId = config.dieId,
            .planeId = pageIndex / metadata.pageCountPerPlane,
            .blockId = (pageIndex % metadata.pageCountPerPlane)
                       / config.pageCountPerBlock,
            .pageId = pageIndex % config.pageCountPerBlock
            // TODO: ...
        };

        dzPageConfig pageConfig = { .ppa = ppa,
                                    .peCycleCountPenalty = peCycleCountPenalty,
                                    .pageSizeInBytes = config.pageSizeInBytes,
                                    .cellType = config.cellType };

        if (!dzPageInitMetadata(pagePtr, pageConfig)) {
            free(result);

            return NULL;
        }

        pageIndex++;
    }

    return result;
}

/* 
    Returns the previous or the next index of 
    the given `blockIndex` in `die`. 
*/
static dzU64 dzDieGetAdjacentBlockIndex(dzDie *die, dzU64 blockIndex) {
    if (die == NULL || blockIndex == DZ_BLOCK_INVALID_ID)
        return DZ_BLOCK_INVALID_ID;

    dzU64 prevBlockIndex = (blockIndex > 1U) ? (blockIndex - 1U)
                                             : DZ_BLOCK_INVALID_ID;

    if (prevBlockIndex != DZ_BLOCK_INVALID_ID) {
        dzBlockMetadata *prevBlockMetadata =
            (dzBlockMetadata *) (((dzByte *) die->metadata.blocks)
                                 + (prevBlockIndex
                                    * dzBlockGetMetadataSize()));

        if (dzBlockGetState(prevBlockMetadata) == DZ_BLOCK_STATE_BAD)
            prevBlockIndex = DZ_BLOCK_INVALID_ID;
    }

    dzU64 nextBlockIndex = (blockIndex < (die->metadata.blockCountPerDie - 1U))
                               ? (blockIndex + 1U)
                               : DZ_BLOCK_INVALID_ID;

    if (nextBlockIndex != DZ_BLOCK_INVALID_ID) {
        dzBlockMetadata *nextBlockMetadata =
            (dzBlockMetadata *) (((dzByte *) die->metadata.blocks)
                                 + (nextBlockIndex
                                    * dzBlockGetMetadataSize()));

        if (dzBlockGetState(nextBlockMetadata) == DZ_BLOCK_STATE_BAD)
            nextBlockIndex = DZ_BLOCK_INVALID_ID;
    }

    // clang-format off

    if (prevBlockIndex == DZ_BLOCK_INVALID_ID) 
        return nextBlockIndex;
    else if (nextBlockIndex == DZ_BLOCK_INVALID_ID)
        return prevBlockIndex;
    else
        return ((dzUtilsRand() & 1U) ? prevBlockIndex : nextBlockIndex);

    // clang-format on
}

/* Initializes all block metadata in `die.` */
static bool dzDieInitBlockMetadata(dzDie *die) {
    if (die == NULL) return false;

    dzPBA pba = dzDieGetFirstPBA(die);

    for (dzU64 i = 0U; i < die->metadata.blockCountPerDie; i++) {
        dzBlockMetadata *blockMetadata =
            (dzBlockMetadata *) (((dzByte *) die->metadata.blocks)
                                 + (i * dzBlockGetMetadataSize()));

        dzBlockConfig blockConfig;

        blockConfig.pba = pba;
        blockConfig.pageCount = die->config.pageCountPerBlock;
        blockConfig.cellType = die->config.cellType;

        if (!dzBlockInitMetadata(blockMetadata, blockConfig)) return false;

        pba = dzDieGetNextPBA(die, pba);
    }

    return true;
}

/* Initializes all plane metadata in `die.` */
static bool dzDieInitPlaneMetadata(dzDie *die) {
    if (die == NULL) return false;

    for (dzU64 i = 0U; i < die->config.planeCountPerDie; i++) {
        dzPlaneMetadata *planeMetadata =
            (dzPlaneMetadata *) (((dzByte *) die->metadata.planes)
                                 + (i * dzPlaneGetMetadataSize()));

        dzPlaneConfig planeConfig;

        planeConfig.planeId = i;
        planeConfig.blockCount = die->config.blockCountPerPlane;

        if (!dzPlaneInitMetadata(planeMetadata, planeConfig)) return false;
    }

    return true;
}

/* Initializes a `die` metadata. */
static bool dzDieInitMetadata(dzDie *die) {
    if (die == NULL) return false;

    die->metadata.blockCountPerDie = die->config.planeCountPerDie
                                     * die->config.blockCountPerPlane;

    die->metadata.pageCountPerDie = die->metadata.blockCountPerDie
                                    * die->config.pageCountPerBlock;

    die->metadata.pageCountPerPlane = die->metadata.pageCountPerDie
                                      / die->config.planeCountPerDie;

    die->metadata.physicalPageSize = die->config.pageSizeInBytes
                                     + dzPageGetMetadataSize();

    die->metadata.physicalBlockSize = die->config.pageCountPerBlock
                                      * die->metadata.physicalPageSize;

    {
        dzUSize totalPlaneMetadataSize = die->config.planeCountPerDie
                                         * dzPlaneGetMetadataSize();

        dzUSize totalBlockMetadataSize = die->metadata.blockCountPerDie
                                         * dzBlockGetMetadataSize();

        dzByte *extraBuffer = malloc(totalPlaneMetadataSize
                                     + totalBlockMetadataSize);

        if (extraBuffer == NULL) return false;

        die->metadata.planes = (dzPlaneMetadata *) extraBuffer;
        die->metadata.blocks = (dzBlockMetadata *) (extraBuffer
                                                    + totalPlaneMetadataSize);

        if (!dzDieInitPlaneMetadata(die) || !dzDieInitBlockMetadata(die)) {
            dzDieRelease(die);

            return false;
        }
    }

    return true;
}

/* ========================================================================> */

/* 
    Returns `true` if `pba` is pointing to the 
    valid physical block address within `die`.
*/
DZ_API_PRIVATE_INLINE dzBool dzDieIsValidPBA(const dzDie *die, dzPBA pba) {
    // clang-format off

    return (die != NULL 
            && pba.dieId == die->config.dieId
            && pba.planeId < die->config.planeCountPerDie
            && pba.blockId < die->config.blockCountPerPlane);

    // clang-format on
}

/* 
    Returns `true` if `ppa` is pointing to the 
    valid physical page address within `die`.
*/
DZ_API_PRIVATE_INLINE dzBool dzDieIsValidPPA(const dzDie *die, dzPPA ppa) {
    // clang-format off

    return (die != NULL
            && ppa.dieId == die->config.dieId
            && ppa.planeId < die->config.planeCountPerDie
            && ppa.blockId < die->config.blockCountPerPlane
            && ppa.pageId < die->config.pageCountPerBlock);

    // clang-format on
}

/* Returns the pointer to a page corresponding to `ppa` in `die`. */
DZ_API_PRIVATE_INLINE dzByte *dzDiePPAToPtr(const dzDie *die, dzPPA ppa) {
    if (!dzDieIsValidPPA(die, ppa)) return NULL;

    dzUSize pageSize = die->metadata.physicalPageSize;
    dzUSize blockSize = (die->config.pageCountPerBlock * pageSize);
    dzUSize planeSize = (die->config.blockCountPerPlane * blockSize);

    return die->buffer
           + ((ppa.planeId * planeSize) + (ppa.blockId * blockSize)
              + (ppa.pageId * pageSize));
}
