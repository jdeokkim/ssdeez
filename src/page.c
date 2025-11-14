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
    dzU32 factoryMarker;
    dzPageState state;
    dzPPA physicalPageAddress;
    dzU64 totalProgramCount;
    dzU64 totalReadCount;
    // dzF64 lastProgramTime;
    // dzF64 lastReadTime;
    dzF64 maxProgramLatency;
    dzF64 maxReadLatency;
    dzU32 maxPeCycles;
    dzU32 peCycles;
    dzCellType cellType;
};

/* Constants ==============================================================> */

/* Average numbers of P/E cycles per page, for each cell type. */
static const dzU32 peCyclesTable[DZ_CELL_TYPE_COUNT_] = {
    [DZ_CELL_TYPE_SLC] = 85000U,
    [DZ_CELL_TYPE_MLC] = 19000U,
    [DZ_CELL_TYPE_TLC] = 2250U,
    [DZ_CELL_TYPE_QLC] = 750U
};

/* Average 'program' latencies for each cell type, in milliseconds. */
static const dzF64 programLatencyTable[DZ_CELL_TYPE_COUNT_] = {
    [DZ_CELL_TYPE_SLC] = 0.85,
    [DZ_CELL_TYPE_MLC] = 2.25,
    [DZ_CELL_TYPE_TLC] = 3.75,
    [DZ_CELL_TYPE_QLC] = 5.25
};

/* Average 'read' latencies for each cell type, in milliseconds. */
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

/* Private Function Prototypes ============================================> */

/* Returns `true` if `cellType` is a valid NAND flash cell type. */
DZ_API_STATIC_INLINE bool dzIsValidCellType(dzCellType cellType);

/* Public Functions =======================================================> */

/* Initializes a page metadata object within the given `pagePtr`. */
dzResult dzPageInitMetadata(dzByte *pagePtr, dzPageConfig config) {
    if (pagePtr == NULL || !dzIsValidCellType(config.cellType)
        || config.pageSizeInBytes == 0U)
        return DZ_RESULT_INVALID_ARGUMENT;

    dzPageMetadata *pageMetadata =
        (dzPageMetadata *) (pagePtr + config.pageSizeInBytes);

    {
        pageMetadata->factoryMarker = 0x12345678;

        pageMetadata->state = DZ_PAGE_STATE_FREE;

        pageMetadata->physicalPageAddress = config.physicalPageAddress;

        pageMetadata->totalProgramCount = 0U;
        pageMetadata->totalReadCount = 0U;

        // pageMetadata->lastProgramTime = 0.0;
        // pageMetadata->lastReadTime = 0.0;

        pageMetadata->cellType = config.cellType;
    }

    {
        dzF64 programLatencyMu = programLatencyTable[pageMetadata->cellType];
        dzF64 programLatencySigma = DZ_PAGE_PROGRAM_LATENCY_STDDEV_RATIO
                                    * programLatencyMu;

        pageMetadata->maxProgramLatency = programLatencyMu
                                          + (3.0 * programLatencySigma);

        dzF64 readLatencyMu = readLatencyTable[pageMetadata->cellType];
        dzF64 readLatencySigma = DZ_PAGE_READ_LATENCY_STDDEV_RATIO
                                 * readLatencyMu;

        pageMetadata->maxReadLatency = readLatencyMu
                                       + (3.0 * readLatencySigma);
    }

    {
        // clang-format off

        pageMetadata->maxPeCycles = (dzU32) dzUtilsGaussian(
            peCyclesTable[config.cellType], 
            DZ_PAGE_PE_CYCLE_COUNT_STDDEV_RATIO 
                * peCyclesTable[config.cellType]
        );

        if (config.endurancePenalty < 0.0) config.endurancePenalty = 0.0;

        pageMetadata->maxPeCycles = (dzU32) (config.endurancePenalty
                                           * pageMetadata->maxPeCycles);

        // clang-format on

        pageMetadata->peCycles = pageMetadata->maxPeCycles;
    }

    return DZ_RESULT_OK;
}

/* Returns the maximum P/E cycles of a page. */
dzU32 dzPageGetMaxPeCycles(const dzByte *pagePtr, dzU32 pageSizeInBytes) {
    if (pagePtr == NULL || pageSizeInBytes == 0U) return 0U;

    dzPageMetadata *pageMetadata = (dzPageMetadata *) (pagePtr
                                                       + pageSizeInBytes);

    return pageMetadata->maxPeCycles;
}

/* Returns the size of `dzPageMetadata`. */
dzUSize dzPageGetMetadataSize(void) {
    return sizeof(dzPageMetadata);
}

/* ========================================================================> */

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

    return pageMetadata->physicalPageAddress;
}

/* Returns the current state of a page. */
dzPageState dzPageGetState(const dzByte *pagePtr, dzU32 pageSizeInBytes) {
    if (pagePtr == NULL) return DZ_PAGE_STATE_UNKNOWN;

    dzPageMetadata *pageMetadata = (dzPageMetadata *) (pagePtr
                                                       + pageSizeInBytes);

    return pageMetadata->state;
}

/* Returns the read latency of a page, in milliseconds. */
dzResult dzPageGetReadLatency(const dzByte *pagePtr,
                              dzU32 pageSizeInBytes,
                              dzF64 *tR) {
    if (pagePtr == NULL || pageSizeInBytes == 0U || tR == NULL)
        return DZ_RESULT_INVALID_ARGUMENT;

    dzPageMetadata *pageMetadata = (dzPageMetadata *) (pagePtr
                                                       + pageSizeInBytes);

    pageMetadata->totalReadCount++;

    {
        dzF64 rawLatency =
            dzUtilsGaussian(readLatencyTable[pageMetadata->cellType],
                            DZ_PAGE_READ_LATENCY_STDDEV_RATIO
                                * readLatencyTable[pageMetadata->cellType]);

        dzF64 maxLatency = pageMetadata->maxReadLatency;

        *tR = dzUtilsClampF64(rawLatency, 0.01, maxLatency);
    }

    return DZ_RESULT_OK;
}

/* Returns the maximum program latency of a page, in milliseconds. */
dzResult dzPageGetMaxProgramLatency(const dzByte *pagePtr,
                                    dzU32 pageSizeInBytes,
                                    dzF64 *tPROG) {
    if (pagePtr == NULL || pageSizeInBytes == 0U || tPROG == NULL)
        return DZ_RESULT_INVALID_ARGUMENT;

    dzPageMetadata *pageMetadata = (dzPageMetadata *) (pagePtr
                                                       + pageSizeInBytes);

    *tPROG = pageMetadata->maxProgramLatency;

    return DZ_RESULT_OK;
}

/* Returns the maximum read latency of a page, in milliseconds. */
dzResult dzPageGetMaxReadLatency(const dzByte *pagePtr,
                                 dzU32 pageSizeInBytes,
                                 dzF64 *tR) {
    if (pagePtr == NULL || pageSizeInBytes == 0U || tR == NULL)
        return DZ_RESULT_INVALID_ARGUMENT;

    dzPageMetadata *pageMetadata = (dzPageMetadata *) (pagePtr
                                                       + pageSizeInBytes);

    *tR = pageMetadata->maxReadLatency;

    return DZ_RESULT_OK;
}

/* ========================================================================> */

/* Returns `true` if the given page is factory-bad. */
dzBool dzPageIsFactoryBad(dzByte *pagePtr, dzU32 pageSizeInBytes) {
    if (pagePtr != NULL && pageSizeInBytes != 0U) return false;

    dzPageMetadata *pageMetadata = (dzPageMetadata *) (pagePtr
                                                       + pageSizeInBytes);

    return (pageMetadata->factoryMarker == 0U);
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

/* Marks a page as factory-bad. */
dzResult dzPageMarkAsFactoryBad(dzByte *pagePtr, dzU32 pageSizeInBytes) {
    if (pagePtr == NULL || pageSizeInBytes == 0U)
        return DZ_RESULT_INVALID_ARGUMENT;

    dzPageMetadata *pageMetadata = (dzPageMetadata *) (pagePtr
                                                       + pageSizeInBytes);

    /*
        NOTE: According to the ONFI 1.0 specification, 
              at least one byte has to be `0x00`
              in order to mark this page as defective
    */
    pageMetadata->factoryMarker = 0U;

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
        || pageMetadata->state == DZ_PAGE_STATE_FREE
        || pageMetadata->state == DZ_PAGE_STATE_RESERVED)
        return DZ_RESULT_INVALID_STATE;

    pageMetadata->peCycles--;

    pageMetadata->state = (pageMetadata->peCycles == 0U) ? DZ_PAGE_STATE_BAD
                                                         : DZ_PAGE_STATE_FREE;

    return DZ_RESULT_OK;
}

/* Marks a page as reserved. */
dzResult dzPageMarkAsReserved(dzByte *pagePtr, dzU32 pageSizeInBytes) {
    if (pagePtr == NULL || pageSizeInBytes == 0U)
        return DZ_RESULT_INVALID_ARGUMENT;

    dzPageMetadata *pageMetadata = (dzPageMetadata *) (pagePtr
                                                       + pageSizeInBytes);

    pageMetadata->state = DZ_PAGE_STATE_RESERVED;

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
                           dzF64 *tPROG) {
    if (pagePtr == NULL || pageSizeInBytes == 0U || tPROG == NULL)
        return DZ_RESULT_INVALID_ARGUMENT;

    dzPageMetadata *pageMetadata = (dzPageMetadata *) (pagePtr
                                                       + pageSizeInBytes);

    if (pageMetadata->state != DZ_PAGE_STATE_FREE)
        return DZ_RESULT_INVALID_STATE;

    pageMetadata->totalProgramCount++;

    pageMetadata->state = DZ_PAGE_STATE_VALID;

    {
        dzF64 rawLatency =
            dzUtilsGaussian(programLatencyTable[pageMetadata->cellType],
                            DZ_PAGE_PROGRAM_LATENCY_STDDEV_RATIO
                                * programLatencyTable[pageMetadata->cellType]);

        dzF64 maxLatency = pageMetadata->maxProgramLatency;

        *tPROG = dzUtilsClampF64(rawLatency, 0.01, maxLatency);
    }

    return DZ_RESULT_OK;
}

/* Private Functions ======================================================> */

/* Returns `true` if `cellType` is a valid NAND flash cell type. */
DZ_API_STATIC_INLINE bool dzIsValidCellType(dzCellType cellType) {
    return (cellType > DZ_CELL_TYPE_UNKNOWN && cellType < DZ_CELL_TYPE_COUNT_);
}
