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

#define dzDieForEachPageInBlock(dieBuffer,                                    \
                                dieMetadata,                                  \
                                blockId,                                      \
                                ptrIdentifier)                                \
    for (dzByte *ptrIdentifier =                                              \
             (dieBuffer) + ((blockId) * ((dieMetadata).physicalBlockSize)),   \
                *ptrIdentifier##__LINE__ =                                    \
                    (dieBuffer)                                               \
                    + (((blockId) + 1U) * ((dieMetadata).physicalBlockSize)); \
         ptrIdentifier < ptrIdentifier##__LINE__;                             \
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

/* Returns the metadata of the `blockId`-th block in `die`. */
static DZ_API_INLINE dzBlockMetadata *dzDieBlockIdToMetadata(const dzDie *die,
                                                             dzU64 blockId);

/* Returns the metadata of the `planeId`-th plane in `die`. */
static DZ_API_INLINE dzPlaneMetadata *dzDiePlaneIdToMetadata(const dzDie *die,
                                                             dzU64 planeId);

/* Creates a die buffer with the given `config` and `metadata`. */
static dzByte *dzDieCreateBuffer(dzDieConfig config, dzDieMetadata metadata);

/* Initializes a `die` metadata. */
static bool dzDieInitMetadata(dzDie *die);

/* ========================================================================> */

/* Returns the block identifier of the `pageId`-th page in `die.` */
static DZ_API_INLINE dzU64 dzDiePageIdToBlockId(const dzDie *die,
                                                dzU64 pageId);

/* Returns the memory address of the `pageId`-th page in `die`. */
static DZ_API_INLINE dzByte *dzDiePageIdToPtr(const dzDie *die, dzU64 pageId);

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
        || (die->buffer = dzDieCreateBuffer(config, die->metadata)) == NULL) {
        dzDieRelease(die);

        return NULL;
    }

    return die;
}

/* Releases the memory allocated for `die`. */
void dzDieRelease(dzDie *die) {
    if (die == NULL) return;

    free(die->metadata.planes), free(die->buffer), free(die);
}

/* Returns the total number of blocks in `die`. */
dzU64 dzDieGetBlockCount(const dzDie *die) {
    return (die != NULL) ? die->metadata.blockCountPerDie : 0U;
}

/* Returns the current state of the `blockId`-th block in `die`. */
dzBlockState dzDieGetBlockState(const dzDie *die, dzU64 blockId) {
    if (die == NULL || blockId >= die->metadata.blockCountPerDie)
        return DZ_BLOCK_STATE_UNKNOWN;

    dzBlockMetadata *blockMetadata = dzDieBlockIdToMetadata(die, blockId);

    return dzBlockGetState(blockMetadata);
}

/* Returns the total number of pages in `die`. */
dzU64 dzDieGetPageCount(const dzDie *die) {
    return (die != NULL) ? die->metadata.pageCountPerDie : 0U;
}

/* Returns the current state of the `pageId`-th page in `die`. */
dzPageState dzDieGetPageState(const dzDie *die, dzU64 pageId) {
    dzByte *pagePtr = dzDiePageIdToPtr(die, pageId);

    if (pagePtr == NULL) return false;

    return dzPageGetState(pagePtr, die->config.pageSizeInBytes);
}

/* Writes `srcBuffer` to the `pageId`-th page in `die`. */
bool dzDieProgramPage(dzDie *die, dzU64 pageId, dzSizedBuffer srcBuffer) {
    dzByte *pagePtr = dzDiePageIdToPtr(die, pageId);

    if (pagePtr == NULL || srcBuffer.ptr == NULL || srcBuffer.size == 0U)
        return false;

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
        dzU64 blockId = dzDiePageIdToBlockId(die, pageId);

        dzBlockMetadata *blockMetadata = dzDieBlockIdToMetadata(die, blockId);

        if (!dzBlockMarkAsValid(blockMetadata)) return false;
    }

    (void) memcpy(pagePtr,
                  srcBuffer.ptr,
                  ((srcBuffer.size < die->config.pageSizeInBytes)
                       ? srcBuffer.size
                       : die->config.pageSizeInBytes));

    return true;
}

/* Reads data from the `pageId`-th page in `die`, copying it to `dstBuffer`. */
bool dzDieReadPage(dzDie *die, dzU64 pageId, dzSizedBuffer dstBuffer) {
    dzByte *pagePtr = dzDiePageIdToPtr(die, pageId);

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

/* Erases the `blockId`-th block in `die`. */
bool dzDieEraseBlock(dzDie *die, dzU64 blockId) {
    if (die == NULL || blockId >= die->metadata.blockCountPerDie) return false;

    bool result = true;

    dzDieForEachPageInBlock(die->buffer, die->metadata, blockId, pagePtr) {
        (void) memset(pagePtr, 0xFF, die->config.pageSizeInBytes);

        if (!dzPageMarkAsFree(pagePtr, die->config.pageSizeInBytes)) {
            result = false;

            break;
        }
    }

    dzBlockMetadata *blockMetadata = dzDieBlockIdToMetadata(die, blockId);

    if (!result) {
        /* NOTE: Mark all pages in the block as bad */

        // clang-format off

        dzDieForEachPageInBlock(die->buffer, die->metadata, blockId, pagePtr)
            ((void) dzPageMarkAsBad(pagePtr, die->config.pageSizeInBytes));

        // clang-format on

        (void) dzBlockMarkAsBad(blockMetadata);

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

    return result;
}

/* Private Functions ======================================================> */

/* Creates a die buffer with the given `config` and `metadata`. */
static dzByte *dzDieCreateBuffer(dzDieConfig config, dzDieMetadata metadata) {
    dzByte *result = NULL;

    dzU64 bufferSize = metadata.pageCountPerDie * metadata.physicalPageSize;

    if ((result = malloc(bufferSize * sizeof *result)) == NULL) return result;

    // NOTE: Simulating the 'erased' state by setting all bits to `1`
    (void) memset(result, 0xFF, config.pageSizeInBytes);

    dzU64 pageId = 0U, centerPageId = metadata.pageCountPerDie >> 1;

    dzDieForEachPage(result, metadata, pagePtr) {
        dzF64 peCycleCountPenalty = 1.0;

        // NOTE: Pages in the top and bottom layers should have lower endurance
        if (metadata.pageCountPerDie > 2) {
            dzU64 distanceFromCenter = (pageId > centerPageId)
                                           ? (pageId - centerPageId)
                                           : (centerPageId - pageId);

            dzF64 penaltyScale = ((dzF64) distanceFromCenter
                                  / (dzF64) centerPageId);

            peCycleCountPenalty -= (penaltyScale
                                    * DZ_PAGE_PE_CYCLE_COUNT_MAX_PENALTY);
        }

        dzPPA physicalPageAddress = {
            .dieId = config.dieId,
            .planeId = pageId / metadata.pageCountPerPlane,
            .blockId = (pageId % metadata.pageCountPerPlane)
                       / config.pageCountPerBlock,
            .pageId = pageId % config.pageCountPerBlock
            // TODO: ...
        };

        dzPageConfig pageConfig = { .physicalPageAddress = physicalPageAddress,
                                    .peCycleCountPenalty = peCycleCountPenalty,
                                    .pageSizeInBytes = config.pageSizeInBytes,
                                    .cellType = config.cellType };

        if (!dzPageInitMetadata(pagePtr, pageConfig)) {
            free(result);

            return NULL;
        }

        pageId++;
    }

    return result;
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

        dzUSize extraBufferSize = totalPlaneMetadataSize
                                  + totalBlockMetadataSize;

        dzByte *extraBuffer = malloc(extraBufferSize);

        if (extraBuffer == NULL) return false;

        die->metadata.planes = (dzPlaneMetadata *) extraBuffer;
        die->metadata.blocks = (dzBlockMetadata *) (extraBuffer
                                                    + totalPlaneMetadataSize);

        for (dzU64 i = 0U; i < die->config.planeCountPerDie; i++) {
            dzPlaneMetadata *planeMetadata = dzDiePlaneIdToMetadata(die, i);

            dzPlaneConfig planeConfig = { .planeId = i };

            if (!dzPlaneInitMetadata(planeMetadata, planeConfig)) {
                dzDieRelease(die);

                return false;
            }
        }

        for (dzU64 i = 0U; i < die->metadata.blockCountPerDie; i++) {
            dzBlockMetadata *blockMetadata = dzDieBlockIdToMetadata(die, i);

            dzBlockConfig blockConfig = { .blockId = i,
                                          .cellType = die->config.cellType };

            if (!dzBlockInitMetadata(blockMetadata, blockConfig)) {
                dzDieRelease(die);

                return false;
            }
        }
    }

    return true;
}

/* ========================================================================> */

/* Returns the metadata of the `blockId`-th block in `die`. */
static DZ_API_INLINE dzBlockMetadata *dzDieBlockIdToMetadata(const dzDie *die,
                                                             dzU64 blockId) {
    return (blockId < die->metadata.blockCountPerDie)
               ? (dzBlockMetadata *) (((dzByte *) die->metadata.blocks)
                                      + (blockId * dzBlockGetMetadataSize()))
               : NULL;
}

/* Returns the metadata of the `planeId`-th plane in `die`. */
static DZ_API_INLINE dzPlaneMetadata *dzDiePlaneIdToMetadata(const dzDie *die,
                                                             dzU64 planeId) {
    return (planeId < die->config.planeCountPerDie)
               ? (dzPlaneMetadata *) (((dzByte *) die->metadata.planes)
                                      + (planeId * dzPlaneGetMetadataSize()))
               : NULL;
}

/* Returns the block identifier of the `pageId`-th page in `die.` */
static DZ_API_INLINE dzU64 dzDiePageIdToBlockId(const dzDie *die,
                                                dzU64 pageId) {
    return (die->config.pageCountPerBlock > 0U)
               ? pageId / die->config.pageCountPerBlock
               : DZ_BLOCK_INVALID_ID;
}

/* Returns the memory address of the `pageId`-th page in `die`. */
static DZ_API_INLINE dzByte *dzDiePageIdToPtr(const dzDie *die, dzU64 pageId) {
    if (die == NULL || die->buffer == NULL
        || pageId >= die->metadata.pageCountPerDie)
        return NULL;

    return die->buffer + (pageId * die->metadata.physicalPageSize);
}
