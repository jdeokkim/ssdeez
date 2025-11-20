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

#include <string.h>

#include "ssdeez.h"

/* Macros =================================================================> */

#define DZ_DIE_FOR_EACH_PAGE(dieBuffer, dieMetadata, ptrIdentifier) \
    for (dzByte *ptrIdentifier = (dieBuffer),                       \
                *ptrIdentifier##__LINE__ =                          \
                    (dieBuffer)                                     \
                    + ((dieMetadata).pageCountPerDie                \
                       * (dieMetadata).physicalPageSize);           \
         ptrIdentifier < ptrIdentifier##__LINE__;                   \
         ptrIdentifier += (dieMetadata).physicalPageSize)

/* Typedefs ===============================================================> */

/* A structure that represents the metadata of a NAND flash die. */
typedef struct dzDieMetadata_ {
    dzU64 pageCountPerDie;
    dzU64 blockCountPerDie;
    dzU64 physicalBlockSize;
    dzU64 physicalPageSize;
    dzU32 maxProgramTime;
    dzU32 maxReadTime;
    dzU32 maxEraseTime;
} dzDieMetadata;

/* A structure that represents various statistics of a NAND flash die. */
typedef struct dzDieStats_ {
    dzF64 totalProgramLatency;
    dzU64 totalProgramCount;
    dzF64 totalReadLatency;
    dzU64 totalReadCount;
    dzF64 totalEraseLatency;
    dzU64 totalEraseCount;
    // TODO: ...
} dzDieStats;

/* A structure that represents a NAND flash die. */
struct dzDie_ {
    dzDieStats stats;
    dzDieMetadata metadata;
    dzDieConfig config;
    dzByte *buffer;
    dzByte status;
};

/* Constants ==============================================================> */

// clang-format off

/* Average "page program time" for each cell type, in microseconds. */
static const dzU32 tPROGTable[DZ_CELL_TYPE_COUNT_] = {
    [DZ_CELL_TYPE_SLC] = 850U,
    [DZ_CELL_TYPE_MLC] = 2250U,
    [DZ_CELL_TYPE_TLC] = 3750U,
    [DZ_CELL_TYPE_QLC] = 5250U
};

/* Average "page read time" for each cell type, in microseconds. */
static const dzU32 tRTable[DZ_CELL_TYPE_COUNT_] = { 
    [DZ_CELL_TYPE_SLC] = 15U,
    [DZ_CELL_TYPE_MLC] = 35U,
    [DZ_CELL_TYPE_TLC] = 60U,
    [DZ_CELL_TYPE_QLC] = 85U
};

/* Average "block erase time" for each cell type, in microseconds. */
static const dzU32 tBERSTable[DZ_CELL_TYPE_COUNT_] = {
    [DZ_CELL_TYPE_SLC] = 2000U,
    [DZ_CELL_TYPE_MLC] = 3000U,
    [DZ_CELL_TYPE_TLC] = 3500U,
    [DZ_CELL_TYPE_QLC] = 4000U
};

// clang-format on

/* ------------------------------------------------------------------------> */

/* Standard deviation ratio for "page program time". */
static const dzF32 tPROGSigmaRatio = 0.01f;

/* Standard deviation ratio for "page read time". */
static const dzF32 tRSigmaRatio = 0.025f;

/* Standard deviation ratio for "block erase time". */
static const dzF32 tBERSSigmaRatio = 0.05f;

/* Private Variables ======================================================> */

// TODO: ...

/* Private Function Prototypes ============================================> */

/* Initializes the metadata of `die`. */
static bool dzDieInitMetadata(dzDie *die);

/* Initializes the data area and the spare area of each page in `die`. */
static bool dzDieInitPages(dzDie *die);

/* ------------------------------------------------------------------------> */

/* Mark a random number of blocks as defective. */
static bool dzDieCorruptRandomBlocks(dzDie *die);

/* Returns the previous or the next index of `blockIndex` within `die`. */
static dzID dzDieGetAdjacentBlockIndex(dzDie *die, dzID blockIndex);

/* ------------------------------------------------------------------------> */

/* Returns `true` if the `blockIndex`-th block within `die` is defective. */
static bool dzDieIsBlockDefective(dzDie *die, dzID blockIndex);

/* Marks the `blockIndex`-th block within `die` as defective. */
static bool dzDieMarkBlockAsDefective(dzDie *die, dzID blockIndex);

/* Public Functions =======================================================> */

/* Initializes `*die` with the given `config`. */
dzResult dzDieInit(dzDie **die, dzDieConfig config) {
    // clang-format off

    if (die == NULL
        || config.dieId == DZ_API_INVALID_ID
        || config.cellType <= DZ_CELL_TYPE_UNKNOWN
        || config.cellType >= DZ_CELL_TYPE_COUNT_
        || config.badBlockRatio >= 1.0
        || config.planeCountPerDie == 0U 
        || config.blockCountPerPlane == 0U
        || config.pageCountPerBlock == 0U
        || config.pageSizeInBytes == 0U)
        return DZ_RESULT_INVALID_ARGUMENT;

    // clang-format on

    if ((config.pageCountPerBlock % 32U) != 0U
        || (config.pageSizeInBytes & 1U) != 0U)
        return DZ_RESULT_INVALID_ARGUMENT;

    dzDie *newDie = malloc(sizeof *newDie);

    if (newDie == NULL) return DZ_RESULT_OUT_OF_MEMORY;

    newDie->stats = (dzDieStats) { .totalProgramLatency = 0.0,
                                   .totalProgramCount = 0U,
                                   .totalReadLatency = 0.0,
                                   .totalReadCount = 0U,
                                   .totalEraseLatency = 0.0,
                                   .totalEraseCount = 0U };

    newDie->config = config;

    if (!dzDieInitMetadata(newDie)) {
        dzDieDeinit(newDie);

        return DZ_RESULT_UNKNOWN;
    }

    if (!dzDieInitPages(newDie)) {
        dzDieDeinit(newDie);

        return DZ_RESULT_OUT_OF_MEMORY;
    }

    if (!dzDieCorruptRandomBlocks(newDie)) {
        dzDieDeinit(newDie);

        return DZ_RESULT_INJECTION_FAILED;
    }

    newDie->status = DZ_DIE_STATUS_RDY;

    *die = newDie;

    return DZ_RESULT_OK;
}

/* Releases the memory allocated for `die`. */
void dzDieDeinit(dzDie *die) {
    if (die == NULL) return;

    free(die->buffer), free(die);
}

/* ------------------------------------------------------------------------> */

/* Private Functions ======================================================> */

/* Initializes the metadata of `die`. */
static bool dzDieInitMetadata(dzDie *die) {
    die->metadata.blockCountPerDie = die->config.planeCountPerDie
                                     * (dzU64) die->config.blockCountPerPlane;

    die->metadata.pageCountPerDie = die->metadata.blockCountPerDie
                                    * die->config.pageCountPerBlock;

    die->metadata.physicalPageSize = die->config.pageSizeInBytes
                                     + (dzU64) dzPageGetSpareAreaSize();

    die->metadata.physicalBlockSize = die->config.pageCountPerBlock
                                      * die->metadata.physicalPageSize;

    {
        dzU32 tPROGMu = tPROGTable[die->config.cellType];
        dzF32 tPROGSigma = tPROGSigmaRatio * (dzF32) tPROGMu;

        if (tPROGSigma <= 0.0f) return false;

        die->metadata.maxProgramTime = tPROGMu + (dzU32) (3.0f * tPROGSigma);
    }

    {
        dzU32 tRMu = tRTable[die->config.cellType];
        dzF32 tRSigma = tRSigmaRatio * (dzF32) tRMu;

        if (tRSigma <= 0.0f) return false;

        die->metadata.maxProgramTime = tRMu + (dzU32) (3.0f * tRSigma);
    }

    {
        dzU32 tBERSMu = tBERSTable[die->config.cellType];
        dzF32 tBERSSigma = tBERSSigmaRatio * (dzF32) tBERSMu;

        if (tBERSSigma <= 0.0f) return false;

        die->metadata.maxProgramTime = tBERSMu + (dzU32) (3.0f * tBERSSigma);
    }

    return true;
}

/* Initializes the data area and the spare area of each page in `die`. */
static bool dzDieInitPages(dzDie *die) {
    const dzDieConfig config = die->config;
    const dzDieMetadata metadata = die->metadata;

    dzU64 bufferSize = metadata.pageCountPerDie * metadata.physicalPageSize;

    dzByte *newBuffer = malloc(bufferSize * sizeof *newBuffer);

    if (newBuffer == NULL) return false;

    die->buffer = newBuffer;

    // NOTE: Simulating the 'factory reset' state by setting all bits to one
    (void) memset(die->buffer, (dzByte) 0xFF, bufferSize);

    DZ_DIE_FOR_EACH_PAGE(die->buffer, metadata, pagePtr) {
        dzPage page = { .ptr = pagePtr, .size = config.pageSizeInBytes };

        if (dzPageInit(page) != DZ_RESULT_OK) return false;
    }

    return true;
}

/* ------------------------------------------------------------------------> */

/* Mark a random number of blocks as defective. */
static bool dzDieCorruptRandomBlocks(dzDie *die) {
    dzU64 blockCountPerDie = die->metadata.blockCountPerDie;

    if (die->config.badBlockRatio <= 0.0) return true;

    dzF64 badBlockCountAsF64 = die->config.badBlockRatio
                               * (dzF64) blockCountPerDie;

    dzU64 badBlockCount = (dzU64) badBlockCountAsF64;

    if ((badBlockCountAsF64 - (dzU64) badBlockCountAsF64) > 0.0)
        badBlockCount++;

    // NOTE: Block #0 is always guaranteed to be a 'good' block
    dzID blockIndex = dzUtilsRandRange(1U, blockCountPerDie - 1U);

    while (badBlockCount > 0U) {
        if (!dzDieMarkBlockAsDefective(die, blockIndex)) return false;

        dzU64 newBlockIndex = dzDieGetAdjacentBlockIndex(die, blockIndex);

        // NOTE: Corruption may or may not spread within an one-block radius
        if ((dzUtilsRand() & 1U) || newBlockIndex == DZ_API_INVALID_ID)
            blockIndex = dzUtilsRandRange(1U, blockCountPerDie - 1U);
        else
            blockIndex = newBlockIndex;

        badBlockCount--;
    }

    return true;
}

/* Returns the previous or the next index of `blockIndex` within `die`. */
static dzID dzDieGetAdjacentBlockIndex(dzDie *die, dzID blockIndex) {
    dzID prevBlockIndex = (blockIndex > 0U) ? (blockIndex - 1U)
                                            : DZ_API_INVALID_ID;

    if ((prevBlockIndex != DZ_API_INVALID_ID)
        && dzDieIsBlockDefective(die, prevBlockIndex))
        prevBlockIndex = DZ_API_INVALID_ID;

    dzID nextBlockIndex = (blockIndex < (die->metadata.blockCountPerDie - 1U))
                              ? (blockIndex + 1U)
                              : DZ_API_INVALID_ID;

    if ((nextBlockIndex != DZ_API_INVALID_ID)
        && dzDieIsBlockDefective(die, nextBlockIndex))
        nextBlockIndex = DZ_API_INVALID_ID;

    // clang-format off

    if (prevBlockIndex == DZ_API_INVALID_ID) 
        return nextBlockIndex;
    else if (nextBlockIndex == DZ_API_INVALID_ID)
        return prevBlockIndex;
    else
        return ((dzUtilsRand() & 1U) ? prevBlockIndex : nextBlockIndex);

    // clang-format on
}

/* ------------------------------------------------------------------------> */

/* Returns `true` if the `blockIndex`-th block within `die` is defective. */
static bool dzDieIsBlockDefective(dzDie *die, dzID blockIndex) {
    if (die == NULL || blockIndex == DZ_API_INVALID_ID) return true;

    dzPage firstPage = {
        .ptr = die->buffer + (blockIndex * die->metadata.physicalBlockSize),
        .size = die->config.pageSizeInBytes
    };

    if (dzPageIsDefective(firstPage)) return true;

    dzPage lastPage = { .ptr = (firstPage.ptr
                                + die->metadata.physicalBlockSize)
                               - die->metadata.physicalPageSize,
                        .size = die->config.pageSizeInBytes };

    if (dzPageIsDefective(lastPage)) return true;

    return false;
}

/* Marks the `blockIndex`-th block within `die` as defective. */
static bool dzDieMarkBlockAsDefective(dzDie *die, dzID blockIndex) {
    dzPage firstPage = {
        .ptr = die->buffer + (blockIndex * die->metadata.physicalBlockSize),
        .size = die->config.pageSizeInBytes
    };

    if (dzPageMarkAsDefective(firstPage) != DZ_RESULT_OK) return false;

    dzPage lastPage = { .ptr = (firstPage.ptr
                                + die->metadata.physicalBlockSize)
                               - die->metadata.physicalPageSize,
                        .size = die->config.pageSizeInBytes };

    if (dzPageMarkAsDefective(lastPage) != DZ_RESULT_OK) return false;

    return true;
}
