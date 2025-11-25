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

/* An enumeration that represents the current state of a NAND flash die. */
typedef enum dzDieState_ {
    DZ_DIE_STATE_IDLE,
    DZ_DIE_STATE_RST_EXECUTE
} dzDieState;

/* ------------------------------------------------------------------------> */

/* A structure that represents the metadata of a NAND flash die. */
typedef struct dzDieMetadata_ {
    dzTimestamp currentTime;
    dzTimestamp remainingTime;
    dzU64 pageCountPerDie;
    dzU32 blockCountPerDie;
    dzU32 physicalBlockSize;
    dzU32 physicalPageSize;
    dzU32 maxProgramTime;
    dzU32 maxReadTime;
    dzU32 maxEraseTime;
} dzDieMetadata;

/* A structure that represents various statistics of a NAND flash die. */
typedef struct dzDieStats_ {
    dzU64 totalProgramLatency;
    dzU64 totalProgramCount;
    dzU64 totalReadLatency;
    dzU64 totalReadCount;
    dzU64 totalEraseLatency;
    dzU64 totalEraseCount;
} dzDieStats;

/* A structure that represents a NAND flash die. */
struct dzDie_ {
    dzDieStats stats;
    dzDieMetadata metadata;
    dzDieConfig config;
    dzByte *buffer;
    dzDieState state;
    dzByte status;
};

/* Constants ==============================================================> */

// clang-format off

/* Average "page program time" for each cell type, in microseconds. */
static const dzU16 tPROGTable[DZ_CELL_TYPE_COUNT_] = {
    [DZ_CELL_TYPE_SLC] = 850U,
    [DZ_CELL_TYPE_MLC] = 2250U,
    [DZ_CELL_TYPE_TLC] = 3750U,
    [DZ_CELL_TYPE_QLC] = 5250U
};

/* Average "page read time" for each cell type, in microseconds. */
static const dzU16 tRTable[DZ_CELL_TYPE_COUNT_] = { 
    [DZ_CELL_TYPE_SLC] = 15U,
    [DZ_CELL_TYPE_MLC] = 35U,
    [DZ_CELL_TYPE_TLC] = 60U,
    [DZ_CELL_TYPE_QLC] = 85U
};

/* 
    Average "reset time" when not performing a "program" 
    or an "erase" operation, for each cell type, in microseconds. 
*/
static const dzU16 tRST0Table[DZ_CELL_TYPE_COUNT_] = {
    [DZ_CELL_TYPE_SLC] = 5U,
    [DZ_CELL_TYPE_MLC] = 8U,
    [DZ_CELL_TYPE_TLC] = 10U,
    [DZ_CELL_TYPE_QLC] = 12U
};

/* 
    Average "reset time" when performing a "program" operation, 
    for each cell type, in microseconds. 
*/
static const dzU16 tRST1Table[DZ_CELL_TYPE_COUNT_] = {
    [DZ_CELL_TYPE_SLC] = 11U,
    [DZ_CELL_TYPE_MLC] = 14U,
    [DZ_CELL_TYPE_TLC] = 18U,
    [DZ_CELL_TYPE_QLC] = 21U
};

/* 
    Average "reset time" when performing an "erase" operation, 
    for each cell type, in microseconds. 
*/
static const dzU16 tRST2Table[DZ_CELL_TYPE_COUNT_] = {
    [DZ_CELL_TYPE_SLC] = 400U,
    [DZ_CELL_TYPE_MLC] = 650U,
    [DZ_CELL_TYPE_TLC] = 725U,
    [DZ_CELL_TYPE_QLC] = 850U
};

/* Average "block erase time" for each cell type, in microseconds. */
static const dzU16 tBERSTable[DZ_CELL_TYPE_COUNT_] = {
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

/* Standard deviation ratio for "reset time". */
static const dzF32 tRSTSigmaRatio = 0.075f;

/* Standard deviation ratio for "block erase time". */
static const dzF32 tBERSSigmaRatio = 0.05f;

/* Private Variables ======================================================> */

// TODO: ...

/* Private Function Prototypes ============================================> */

/* Initializes the metadata of `die`. */
static dzBool dzDieInitMetadata(dzDie *die);

/* Initializes the data area and the spare area of each page in `die`. */
static dzBool dzDieInitPages(dzDie *die);

/* ------------------------------------------------------------------------> */

/* Mark a random number of blocks as defective. */
static dzBool dzDieCorruptRandomBlocks(dzDie *die);

/* Creates an ONFI parameter page section on the first block of `die`.  */
static dzBool dzDieCreateParameterPages(dzDie *die);

/* Returns the previous or the next index of `blockIndex` within `die`. */
static dzID dzDieGetAdjacentBlockIndex(dzDie *die, dzID blockIndex);

/* ------------------------------------------------------------------------> */

/* Returns `true` if the `blockIndex`-th block within `die` is defective. */
static dzBool dzDieIsBlockDefective(dzDie *die, dzID blockIndex);

/* Marks the `blockIndex`-th block within `die` as defective. */
static dzBool dzDieMarkBlockAsDefective(dzDie *die, dzID blockIndex);

/* ------------------------------------------------------------------------> */

/* Performs a "Reset" operation on `die`. */
static void dzDieReset(dzDie *die);

/* Public Functions =======================================================> */

/* Initializes `*die` with the given `config`. */
dzResult dzDieInit(dzDie **die, dzDieConfig config) {
    // clang-format off

    if (die == NULL
        || config.dieId == DZ_API_INVALID_ID
        || config.cellType <= DZ_CELL_TYPE_UNKNOWN
        || config.cellType >= DZ_CELL_TYPE_COUNT_
        || config.badBlockRatio >= 1.0f
        || config.planeCountPerDie == 0U 
        || config.blockCountPerPlane == 0U
        || config.pageCountPerBlock == 0U
        || config.pageSizeInBytes == 0U)
        return DZ_RESULT_INVALID_ARGUMENT;

    // clang-format on

    if ((config.pageCountPerBlock % 32U) != 0U) {
        DZ_API_ERROR("`pageCountPerBlock` should be a multiple of 32\n");

        return DZ_RESULT_INVALID_ARGUMENT;
    }

    if ((config.pageSizeInBytes & (config.pageSizeInBytes - 1U)) != 0U) {
        DZ_API_ERROR("`pageSizeInBytes` should be a power of 2\n");

        return DZ_RESULT_INVALID_ARGUMENT;
    }

    if (config.pageSizeInBytes < 512U) {
        DZ_API_ERROR("`pageSizeInBytes` should be greater than 512\n");

        return DZ_RESULT_INVALID_ARGUMENT;
    }

    dzDie *newDie = malloc(sizeof *newDie);

    if (newDie == NULL) return DZ_RESULT_OUT_OF_MEMORY;

    if (config.isVerbose) DZ_API_INFO("initializing die #%lu\n", config.dieId);

    newDie->stats = (dzDieStats) { .totalProgramLatency = 0U,
                                   .totalProgramCount = 0U,
                                   .totalReadLatency = 0U,
                                   .totalReadCount = 0U,
                                   .totalEraseLatency = 0U,
                                   .totalEraseCount = 0U };

    newDie->config = config;

    newDie->metadata.currentTime = 0U;
    newDie->metadata.remainingTime = 0U;

    if (!dzDieInitMetadata(newDie)) {
        dzDieDeinit(newDie);

        return DZ_RESULT_UNKNOWN;
    }

    if (!dzDieInitPages(newDie)) {
        dzDieDeinit(newDie);

        return DZ_RESULT_OUT_OF_MEMORY;
    }

    if (!dzDieCreateParameterPages(newDie)) {
        dzDieDeinit(newDie);

        return DZ_RESULT_UNKNOWN;
    }

    if (!dzDieCorruptRandomBlocks(newDie)) {
        dzDieDeinit(newDie);

        return DZ_RESULT_INJECTION_FAILED;
    }

    newDie->state = DZ_DIE_STATE_IDLE;
    newDie->status = DZ_DIE_STATUS_RDY;

    *die = newDie;

    return DZ_RESULT_OK;
}

/* Releases the memory allocated for `die`. */
void dzDieDeinit(dzDie *die) {
    if (die == NULL) return;

    if (die->config.isVerbose)
        DZ_API_INFO("deinitializing die #%lu\n", die->config.dieId);

    free(die->buffer), free(die);
}

/* Performs `command` on `die`. */
void dzDieDecodeCommand(dzDie *die, dzByte command, dzTimestamp ts) {
    if (die == NULL || die->metadata.currentTime >= ts) return;

    if (die->config.isVerbose)
        DZ_API_INFO("decoded ONFI command 0x%02X\n", command);

    dzBool isAcceptableWhileBusy = (command == DZ_CHIP_CMD_READ_STATUS)
                                   || (command == DZ_CHIP_CMD_RESET);

    if (!isAcceptableWhileBusy && !(die->status & DZ_DIE_STATUS_RDY)) {
        if (die->config.isVerbose)
            DZ_API_INFO("die #%lu is not ready yet\n", die->config.dieId);

        dzTimestamp elapsedTime = ts - die->metadata.currentTime;

        if (elapsedTime < die->metadata.remainingTime) {
            die->state = DZ_DIE_STATE_IDLE;

            dzDieReset(die);
        }

        die->metadata.currentTime = ts;

        return;
    }

    switch (command) {
        case DZ_CHIP_CMD_RESET:
            dzDieReset(die);

            break;

        default:
            DZ_API_UNIMPLEMENTED();
    }
}

/* Waits until the `die`'s "RDY" status bit is set. */
dzTimestamp dzDieWaitUntilReady(dzDie *die) {
    if (die == NULL || die->status & DZ_DIE_STATUS_RDY) return 0U;

    dzTimestamp result = die->metadata.remainingTime;

    if (die->config.isVerbose)
        DZ_API_INFO("die #%lu: waiting for %lu us until ready\n",
                    die->config.dieId,
                    result);

    switch (die->state) {
        case DZ_DIE_STATE_RST_EXECUTE:
            dzDieReset(die);

            break;

        default:
            DZ_API_UNIMPLEMENTED();
    }

    die->metadata.currentTime += result;

    return result;
}

/* ------------------------------------------------------------------------> */

/* Returns the "Ready" status bit of `die`. */
dzByte dzDieGetRDY(const dzDie *die) {
    return (die != NULL) ? !!(die->status & DZ_DIE_STATUS_RDY) : 0U;
}

/* Private Functions ======================================================> */

/* Initializes the metadata of `die`. */
static dzBool dzDieInitMetadata(dzDie *die) {
    die->metadata.blockCountPerDie = die->config.planeCountPerDie
                                     * (dzU32) die->config.blockCountPerPlane;

    die->metadata.pageCountPerDie = die->metadata.blockCountPerDie
                                    * die->config.pageCountPerBlock;

    die->metadata.physicalPageSize = die->config.pageSizeInBytes
                                     + (dzU32) dzPageGetSpareAreaSize();

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
static dzBool dzDieInitPages(dzDie *die) {
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
static dzBool dzDieCorruptRandomBlocks(dzDie *die) {
    dzU32 blockCountPerDie = die->metadata.blockCountPerDie;

    if (die->config.badBlockRatio <= 0.0f) return true;

    dzF32 badBlockCountAsF32 = die->config.badBlockRatio
                               * (dzF32) blockCountPerDie;

    dzU32 badBlockCount = (dzU32) badBlockCountAsF32;

    if ((badBlockCountAsF32 - (dzU32) badBlockCountAsF32) > 0.0f)
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

/* Creates an ONFI parameter page section on the first block of `die`.  */
static dzBool dzDieCreateParameterPages(dzDie *die) {
    DZ_API_UNUSED_VARIABLE(die);

    // DZ_API_UNIMPLEMENTED();

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
static dzBool dzDieIsBlockDefective(dzDie *die, dzID blockIndex) {
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
static dzBool dzDieMarkBlockAsDefective(dzDie *die, dzID blockIndex) {
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

/* ------------------------------------------------------------------------> */

/* Performs a "Reset" operation on `die`. */
static void dzDieReset(dzDie *die) {
    if (die->state != DZ_DIE_STATE_RST_EXECUTE) {
        die->status &= (dzByte) ~DZ_DIE_STATUS_RDY;

        if (die->config.isVerbose)
            DZ_API_INFO("cleared die #%lu's RDY status bit\n",
                        die->config.dieId);

        die->state = DZ_DIE_STATE_RST_EXECUTE;

        // TODO: ...
        die->metadata.remainingTime = (dzU64)
            dzUtilsGaussian((dzF64) tRST0Table[die->config.cellType],
                            tRSTSigmaRatio);

        DZ_API_UNUSED_VARIABLE(tRST1Table);
        DZ_API_UNUSED_VARIABLE(tRST2Table);
    } else {
        die->status &= (dzByte) ~(DZ_DIE_STATUS_FAIL | DZ_DIE_STATUS_FAILC);
        die->status |= DZ_DIE_STATUS_RDY;

        if (die->config.isVerbose)
            DZ_API_INFO("set die #%lu's RDY status bit\n", die->config.dieId);

        die->state = DZ_DIE_STATE_IDLE;

        die->metadata.remainingTime = 0U;
    }
}
