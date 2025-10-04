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
    dzPageState state;
    dzCellType cellType;
    dzU64 totalProgramCount;
    dzU64 totalReadCount;
    dzU32 peCycleCount;
    // TODO: ...
};

/* Private Function Prototypes ============================================> */

// TODO: ...

/* Constants ==============================================================> */

/* Average numbers of P/E cycles per page, for each cell type. */
static const dzU32 peCycleCountTable[DZ_CELL_TYPE_COUNT_] = {
    [DZ_CELL_TYPE_SLC] = 85000U,
    [DZ_CELL_TYPE_MLC] = 19000U,
    [DZ_CELL_TYPE_TLC] = 2250U,
    [DZ_CELL_TYPE_QLC] = 750U
};

/* Average 'program' latencies for each cell type, in miliseconds. */
static const dzF32 programLatencyTable[DZ_CELL_TYPE_COUNT_] = {
    [DZ_CELL_TYPE_SLC] = 1.0f,
    [DZ_CELL_TYPE_MLC] = 2.5f,
    [DZ_CELL_TYPE_TLC] = 3.75f,
    [DZ_CELL_TYPE_QLC] = 5.5f
};

/* Average 'read' latencies for each cell type, in miliseconds. */
static const dzF32 readLatencyTable[DZ_CELL_TYPE_COUNT_] = {
    [DZ_CELL_TYPE_SLC] = 0.015f,
    [DZ_CELL_TYPE_MLC] = 0.035f,
    [DZ_CELL_TYPE_TLC] = 0.060f,
    [DZ_CELL_TYPE_QLC] = 0.095f
};

/* Private Variables ======================================================> */

// TODO: ...

/* Public Functions =======================================================> */

/* Initializes a page metadata object within the given `pageBuffer`. */
bool dzPageInitMetadata(dzByte *pageBuffer, dzPageConfig config) {
    if (pageBuffer == NULL || !dzIsValidCellType(config.cellType)
        || config.pageSizeInBytes == 0U)
        return false;

    dzPageMetadata *pageMetadata =
        (dzPageMetadata *) (pageBuffer + config.pageSizeInBytes);

    pageMetadata->state = DZ_PAGE_STATE_FREE;

    pageMetadata->cellType = config.cellType;

    pageMetadata->totalProgramCount = 0U;
    pageMetadata->totalReadCount = 0U;

    {
        // clang-format off

        pageMetadata->peCycleCount = dzUtilsGaussian(
            peCycleCountTable[config.cellType],
            DZ_PAGE_PE_CYCLE_COUNT_STDDEV_RATIO
                * peCycleCountTable[config.cellType]
        );

        // clang-format on

        if (config.peCycleCountPenalty < 0.0f)
            config.peCycleCountPenalty = 0.0f;

        pageMetadata->peCycleCount *= config.peCycleCountPenalty;
    }

    return true;
}

/* Returns the size of `dzPageMetadata`. */
dzUSize dzPageGetMetadataSize(void) {
    return sizeof(dzPageMetadata);
}

/* Marks a page as valid. */
bool dzPageMarkAsValid(dzByte *pageBuffer,
                       dzU32 pageSizeInBytes,
                       dzF32 *outLatency) {
    if (pageBuffer == NULL || pageSizeInBytes == 0U || outLatency == NULL)
        return false;

    dzPageMetadata *pageMetadata = (dzPageMetadata *) (pageBuffer
                                                       + pageSizeInBytes);

    if (pageMetadata->state != DZ_PAGE_STATE_FREE) return false;

    pageMetadata->totalProgramCount++;

    pageMetadata->state = DZ_PAGE_STATE_VALID;

    // NOTE: P/E Latency
    *outLatency = programLatencyTable[pageMetadata->cellType];

    return true;
}

/* Marks a page as free. */
bool dzPageMarkAsFree(dzByte *pageBuffer,
                      dzU32 pageSizeInBytes,
                      dzF32 *outLatency) {
    if (pageBuffer == NULL || pageSizeInBytes == 0U || outLatency == NULL)
        return false;

    dzPageMetadata *pageMetadata = (dzPageMetadata *) (pageBuffer
                                                       + pageSizeInBytes);

    if (pageMetadata->state == DZ_PAGE_STATE_CORRUPTED
        || pageMetadata->state == DZ_PAGE_STATE_FREE)
        return false;

    pageMetadata->peCycleCount--;

    pageMetadata->state = (pageMetadata->peCycleCount == 0)
                              ? DZ_PAGE_STATE_CORRUPTED
                              : DZ_PAGE_STATE_FREE;

    // NOTE: P/E Latency
    *outLatency = readLatencyTable[pageMetadata->cellType];

    return true;
}

/* Private Functions ======================================================> */

// TODO: ...
