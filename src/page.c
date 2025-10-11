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

/* Includes ===============================================================> */

#include "ssdeez.h"

/* Macros =================================================================> */

// TODO: ...

/* Typedefs ===============================================================> */

/* A structure that represents the metadata of a NAND flash page. */
struct dzPageMetadata_ {
    dzU64 totalProgramCount;
    dzU64 totalReadCount;
    /// dzF64 lastProgramTime;
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
    [DZ_CELL_TYPE_SLC] = 1.0,
    [DZ_CELL_TYPE_MLC] = 2.5,
    [DZ_CELL_TYPE_TLC] = 3.75,
    [DZ_CELL_TYPE_QLC] = 5.5
};

/* Average 'read' latencies for each cell type, in miliseconds. */
static const dzF64 readLatencyTable[DZ_CELL_TYPE_COUNT_] = {
    [DZ_CELL_TYPE_SLC] = 0.015,
    [DZ_CELL_TYPE_MLC] = 0.035,
    [DZ_CELL_TYPE_TLC] = 0.060,
    [DZ_CELL_TYPE_QLC] = 0.095
};

/* ========================================================================> */

/* A constant that represents an invalid page identifier. */
const dzU64 DZ_PAGE_INVALID_ID = UINT64_MAX;

/* Private Variables ======================================================> */

// TODO: ...

/* Private Function Prototypes ============================================> */

/* Returns `true` if `cellType` is a valid NAND flash cell type. */
static DZ_API_INLINE bool dzIsValidCellType(dzCellType cellType);

/* Public Functions =======================================================> */

/* Initializes a page metadata object within the given `pagePtr`. */
bool dzPageInitMetadata(dzByte *pagePtr, dzPageConfig config) {
    if (pagePtr == NULL || !dzIsValidCellType(config.cellType)
        || config.pageSizeInBytes == 0U)
        return false;

    dzPageMetadata *pageMetadata =
        (dzPageMetadata *) (pagePtr + config.pageSizeInBytes);

    {
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

    return true;
}

/* Returns the size of `dzPageMetadata`. */
dzUSize dzPageGetMetadataSize(void) {
    return sizeof(dzPageMetadata);
}

/* Returns the read latency of a page. */
bool dzPageGetReadLatency(const dzByte *pagePtr,
                          dzU32 pageSizeInBytes,
                          dzF64 *readLatency) {
    if (pagePtr == NULL || readLatency == NULL) return false;

    dzPageMetadata *pageMetadata = (dzPageMetadata *) (pagePtr
                                                       + pageSizeInBytes);

    *readLatency =
        dzUtilsGaussian(readLatencyTable[pageMetadata->cellType],
                        DZ_PAGE_READ_LATENCY_STDDEV_RATIO
                            * readLatencyTable[pageMetadata->cellType]);

    return true;
}

/* Marks a page as corrupted. */
bool dzPageMarkAsCorrupted(dzByte *pagePtr, dzU32 pageSizeInBytes) {
    if (pagePtr == NULL || pageSizeInBytes == 0U) return false;

    dzPageMetadata *pageMetadata = (dzPageMetadata *) (pagePtr
                                                       + pageSizeInBytes);

    // NOTE: Free blocks can never be corrupted
    if (pageMetadata->state == DZ_PAGE_STATE_FREE) return false;

    pageMetadata->state = DZ_PAGE_STATE_CORRUPTED;

    return true;
}

/* Marks a page as free. */
bool dzPageMarkAsFree(dzByte *pagePtr, dzU32 pageSizeInBytes) {
    if (pagePtr == NULL || pageSizeInBytes == 0U) return false;

    dzPageMetadata *pageMetadata = (dzPageMetadata *) (pagePtr
                                                       + pageSizeInBytes);

    if (pageMetadata->state == DZ_PAGE_STATE_CORRUPTED
        || pageMetadata->state == DZ_PAGE_STATE_FREE)
        return false;

    pageMetadata->peCycleCount--;

    pageMetadata->state = (pageMetadata->peCycleCount == 0)
                              ? DZ_PAGE_STATE_CORRUPTED
                              : DZ_PAGE_STATE_FREE;

    return true;
}

/* Marks a page as valid. */
bool dzPageMarkAsValid(dzByte *pagePtr,
                       dzU32 pageSizeInBytes,
                       dzF64 *programLatency) {
    if (pagePtr == NULL || pageSizeInBytes == 0U || programLatency == NULL)
        return false;

    dzPageMetadata *pageMetadata = (dzPageMetadata *) (pagePtr
                                                       + pageSizeInBytes);

    if (pageMetadata->state != DZ_PAGE_STATE_FREE) return false;

    pageMetadata->totalProgramCount++;

    pageMetadata->state = DZ_PAGE_STATE_VALID;

    *programLatency =
        dzUtilsGaussian(programLatencyTable[pageMetadata->cellType],
                        DZ_PAGE_PROGRAM_LATENCY_STDDEV_RATIO
                            * programLatencyTable[pageMetadata->cellType]);

    return true;
}

/* Private Functions ======================================================> */

/* Returns `true` if `cellType` is a valid NAND flash cell type. */
static DZ_API_INLINE bool dzIsValidCellType(dzCellType cellType) {
    return (cellType > DZ_CELL_TYPE_UNKNOWN && cellType < DZ_CELL_TYPE_COUNT_);
}
