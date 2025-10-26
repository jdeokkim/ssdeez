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

#include "ssdeez.h"

/* Macros =================================================================> */

// TODO: ...

/* Typedefs ===============================================================> */

/* A structure that represents the metadata of a NAND flash page. */
struct dzPageMetadata_ {
    dzPPA ppa;
    dzU64 totalProgramCount;
    dzU64 totalReadCount;
    // dzF64 lastProgramTime;
    // dzF64 lastReadTime;
    dzU32 peCycleCount;
    dzCellType cellType;
    dzPageState state;
    // TODO: ...
};

/* Constants ==============================================================> */

/* Average numbers of P/E cycles per page, for each cell type. */
static const dzU32 peCycleCountTable[DZ_CELL_TYPE_COUNT_] = {
    [DZ_CELL_TYPE_SLC] = 85000U,
    [DZ_CELL_TYPE_MLC] = 19000U,
    [DZ_CELL_TYPE_TLC] = 2250U,
    [DZ_CELL_TYPE_QLC] = 750U
};

/* Average 'program' latencies for each cell type, in miliseconds. */
static const dzF64 programLatencyTable[DZ_CELL_TYPE_COUNT_] = {
    [DZ_CELL_TYPE_SLC] = 0.85,
    [DZ_CELL_TYPE_MLC] = 2.25,
    [DZ_CELL_TYPE_TLC] = 3.75,
    [DZ_CELL_TYPE_QLC] = 5.25
};

/* Average 'read' latencies for each cell type, in miliseconds. */
static const dzF64 readLatencyTable[DZ_CELL_TYPE_COUNT_] = {
    [DZ_CELL_TYPE_SLC] = 0.015,
    [DZ_CELL_TYPE_MLC] = 0.035,
    [DZ_CELL_TYPE_TLC] = 0.060,
    [DZ_CELL_TYPE_QLC] = 0.085
};

/* ========================================================================> */

/* A constant that represents an invalid page identifier. */
const dzU64 DZ_PAGE_INVALID_ID = UINT64_MAX;

/* Private Variables ======================================================> */

// TODO: ...

/* Private Function Prototypes ============================================> */

/* Returns `true` if `cellType` is a valid NAND flash cell type. */
DZ_API_PRIVATE_INLINE bool dzIsValidCellType(dzCellType cellType);

/* Public Functions =======================================================> */

/* Initializes a page metadata object within the given `pagePtr`. */
dzResult dzPageInitMetadata(dzByte *pagePtr, dzPageConfig config) {
    if (pagePtr == NULL || !dzIsValidCellType(config.cellType)
        || config.pageSizeInBytes == 0U)
        return DZ_RESULT_INVALID_ARGUMENT;

    dzPageMetadata *pageMetadata =
        (dzPageMetadata *) (pagePtr + config.pageSizeInBytes);

    {
        pageMetadata->ppa = config.ppa;

        pageMetadata->totalProgramCount = 0U;
        pageMetadata->totalReadCount = 0U;

        // pageMetadata->lastProgramTime = 0.0;
        // pageMetadata->lastReadTime = 0.0;

        pageMetadata->cellType = config.cellType;

        pageMetadata->state = DZ_PAGE_STATE_FREE;
    }

    {
        // clang-format off

        pageMetadata->peCycleCount = (dzU32) dzUtilsGaussian(
            peCycleCountTable[config.cellType], 
            DZ_PAGE_PE_CYCLE_COUNT_STDDEV_RATIO 
                * peCycleCountTable[config.cellType]
        );

        // clang-format on

        if (config.peCycleCountPenalty < 0.0) config.peCycleCountPenalty = 0.0;

        pageMetadata->peCycleCount = (dzU32) (config.peCycleCountPenalty
                                              * pageMetadata->peCycleCount);
    }

    return DZ_RESULT_OK;
}

/* Returns the size of `dzPageMetadata`. */
dzUSize dzPageGetMetadataSize(void) {
    return sizeof(dzPageMetadata);
}

/* Returns the physical page address of a page. */
dzPPA dzPageGetPPA(const dzByte *pagePtr, dzU32 pageSizeInBytes) {
    if (pagePtr == NULL || pageSizeInBytes == 0U)
        return (dzPPA) { .chipId = DZ_CHIP_INVALID_ID,
                         .dieId = DZ_DIE_INVALID_ID,
                         .planeId = DZ_PLANE_INVALID_ID,
                         .blockId = DZ_BLOCK_INVALID_ID,
                         .pageId = DZ_PAGE_INVALID_ID };

    dzPageMetadata *pageMetadata = (dzPageMetadata *) (pagePtr
                                                       + pageSizeInBytes);

    return pageMetadata->ppa;
}

/* Returns the current state of a page. */
dzPageState dzPageGetState(const dzByte *pagePtr, dzU32 pageSizeInBytes) {
    if (pagePtr == NULL) return DZ_PAGE_STATE_UNKNOWN;

    dzPageMetadata *pageMetadata = (dzPageMetadata *) (pagePtr
                                                       + pageSizeInBytes);

    return pageMetadata->state;
}

/* Returns the read latency of a page. */
dzResult dzPageGetReadLatency(const dzByte *pagePtr,
                              dzU32 pageSizeInBytes,
                              dzF64 *readLatency) {
    if (pagePtr == NULL || pageSizeInBytes == 0U || readLatency == NULL)
        return DZ_RESULT_INVALID_ARGUMENT;

    dzPageMetadata *pageMetadata = (dzPageMetadata *) (pagePtr
                                                       + pageSizeInBytes);

    pageMetadata->totalReadCount++;

    *readLatency =
        dzUtilsGaussian(readLatencyTable[pageMetadata->cellType],
                        DZ_PAGE_READ_LATENCY_STDDEV_RATIO
                            * readLatencyTable[pageMetadata->cellType]);

    return DZ_RESULT_OK;
}

/* Marks a page as bad. */
dzResult dzPageMarkAsBad(dzByte *pagePtr, dzU32 pageSizeInBytes) {
    if (pagePtr == NULL || pageSizeInBytes == 0U)
        return DZ_RESULT_INVALID_ARGUMENT;

    dzPageMetadata *pageMetadata = (dzPageMetadata *) (pagePtr
                                                       + pageSizeInBytes);

    // NOTE: Free blocks can never be corrupted
    if (pageMetadata->state == DZ_PAGE_STATE_FREE)
        return DZ_RESULT_INVALID_STATE;

    pageMetadata->state = DZ_PAGE_STATE_BAD;

    return DZ_RESULT_OK;
}

/* Marks a page as free. */
dzResult dzPageMarkAsFree(dzByte *pagePtr, dzU32 pageSizeInBytes) {
    if (pagePtr == NULL || pageSizeInBytes == 0U)
        return DZ_RESULT_INVALID_ARGUMENT;

    dzPageMetadata *pageMetadata = (dzPageMetadata *) (pagePtr
                                                       + pageSizeInBytes);

    if (pageMetadata->state == DZ_PAGE_STATE_BAD
        || pageMetadata->state == DZ_PAGE_STATE_FREE)
        return DZ_RESULT_INVALID_STATE;

    pageMetadata->peCycleCount--;

    pageMetadata->state = (pageMetadata->peCycleCount == 0)
                              ? DZ_PAGE_STATE_BAD
                              : DZ_PAGE_STATE_FREE;

    return DZ_RESULT_OK;
}

/* Marks a page as unknown. */
dzResult dzPageMarkAsUnknown(dzByte *pagePtr, dzU32 pageSizeInBytes) {
    if (pagePtr == NULL || pageSizeInBytes == 0U)
        return DZ_RESULT_INVALID_ARGUMENT;

    dzPageMetadata *pageMetadata = (dzPageMetadata *) (pagePtr
                                                       + pageSizeInBytes);

    pageMetadata->state = DZ_PAGE_STATE_UNKNOWN;

    return DZ_RESULT_OK;
}

/* Marks a page as valid. */
dzResult dzPageMarkAsValid(dzByte *pagePtr,
                           dzU32 pageSizeInBytes,
                           dzF64 *programLatency) {
    if (pagePtr == NULL || pageSizeInBytes == 0U || programLatency == NULL)
        return DZ_RESULT_INVALID_ARGUMENT;

    dzPageMetadata *pageMetadata = (dzPageMetadata *) (pagePtr
                                                       + pageSizeInBytes);

    if (pageMetadata->state != DZ_PAGE_STATE_FREE)
        return DZ_RESULT_INVALID_STATE;

    pageMetadata->totalProgramCount++;

    pageMetadata->state = DZ_PAGE_STATE_VALID;

    *programLatency =
        dzUtilsGaussian(programLatencyTable[pageMetadata->cellType],
                        DZ_PAGE_PROGRAM_LATENCY_STDDEV_RATIO
                            * programLatencyTable[pageMetadata->cellType]);

    return DZ_RESULT_OK;
}

/* Private Functions ======================================================> */

/* Returns `true` if `cellType` is a valid NAND flash cell type. */
DZ_API_PRIVATE_INLINE bool dzIsValidCellType(dzCellType cellType) {
    return (cellType > DZ_CELL_TYPE_UNKNOWN && cellType < DZ_CELL_TYPE_COUNT_);
}
