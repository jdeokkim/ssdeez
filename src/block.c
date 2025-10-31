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

// TODO: ...

/* Typedefs ===============================================================> */

/* A structure that represents the metadata of a NAND flash block. */
struct dzBlockMetadata_ {
    dzByte *pageStateMap;
    dzPBA pba;
    dzU64 pageCount;
    dzU64 nextPageId;
    dzU64 totalEraseCount;
    // dzF64 lastEraseTime;
    dzCellType cellType;
    dzBlockState state;
    // TODO: ...
};

/* Constants ==============================================================> */

/* Average 'erase' latencies for each cell type, in miliseconds. */
static const dzF64 eraseLatencyTable[DZ_CELL_TYPE_COUNT_] = {
    [DZ_CELL_TYPE_SLC] = 2.0,
    [DZ_CELL_TYPE_MLC] = 3.0,
    [DZ_CELL_TYPE_TLC] = 3.5,
    [DZ_CELL_TYPE_QLC] = 4.0
};

/* A constant that represents an invalid block identifier. */
const dzU64 DZ_BLOCK_INVALID_ID = UINT64_MAX;

/* Private Variables ======================================================> */

// TODO: ...

/* Private Function Prototypes ============================================> */

// TODO: ...

/* Public Functions =======================================================> */

/* Initializes a block metadata within the given `metadata` region. */
dzResult dzBlockInitMetadata(dzBlockMetadata *metadata, dzBlockConfig config) {
    if (metadata == NULL) return DZ_RESULT_INVALID_ARGUMENT;

    {
        metadata->pageStateMap = malloc(config.pageCount
                                        * sizeof *(metadata->pageStateMap));

        for (dzU64 i = 0U; i < config.pageCount; i++)
            metadata->pageStateMap[i] = DZ_PAGE_STATE_FREE;
    }

    {
        metadata->pba = config.pba;

        metadata->pageCount = config.pageCount;
        metadata->nextPageId = 0U;

        metadata->totalEraseCount = 0U;
        // metadata->lastEraseTime = 0.0;

        metadata->cellType = config.cellType;

        metadata->state = DZ_BLOCK_STATE_FREE;
    }

    return DZ_RESULT_OK;
}

/* De-initializes the block `metadata`. */
void dzBlockDeinitMetadata(dzBlockMetadata *metadata) {
    if (metadata == NULL) return;

    free(metadata->pageStateMap);
}

/* Returns the size of `dzBlockMetadata`. */
dzUSize dzBlockGetMetadataSize(void) {
    return sizeof(dzBlockMetadata);
}

/* Writes the next page identifier of a block to `nextPageId`. */
dzResult dzBlockGetNextPageId(dzBlockMetadata *metadata, dzU64 *nextPageId) {
    if (metadata == NULL || nextPageId == NULL)
        return DZ_RESULT_INVALID_ARGUMENT;

    *nextPageId = metadata->nextPageId;

    return DZ_RESULT_OK;
}

/* Returns the physical block address of a block. */
dzPBA dzBlockGetPBA(const dzBlockMetadata *metadata) {
    if (metadata == NULL)
        return (dzPPA) { .chipId = DZ_CHIP_INVALID_ID,
                         .dieId = DZ_DIE_INVALID_ID,
                         .planeId = DZ_PLANE_INVALID_ID,
                         .blockId = DZ_BLOCK_INVALID_ID,
                         .pageId = DZ_PAGE_INVALID_ID };

    return metadata->pba;
}

/* Returns the current state of a block. */
dzBlockState dzBlockGetState(const dzBlockMetadata *metadata) {
    return (metadata != NULL) ? metadata->state : DZ_BLOCK_STATE_UNKNOWN;
}

/* Returns the number of valid pages in a block. */
dzU64 dzBlockGetValidPageCount(const dzBlockMetadata *metadata) {
    if (metadata == NULL) return 0U;

    dzU64 validPageCount = 0U;

    for (dzU64 i = 0U; i < metadata->pageCount; i++)
        validPageCount += (metadata->pageStateMap[i] == DZ_PAGE_STATE_VALID);

    return validPageCount;
}

/* Advances the next page identifier of a block. */
dzResult dzBlockAdvanceNextPageId(dzBlockMetadata *metadata) {
    if (metadata == NULL) return DZ_RESULT_INVALID_ARGUMENT;

    metadata->nextPageId++;

    if (metadata->nextPageId >= metadata->pageCount)
        metadata->nextPageId = DZ_PAGE_INVALID_ID;

    return DZ_RESULT_OK;
}

/* Marks a block as active. */
dzResult dzBlockMarkAsActive(dzBlockMetadata *metadata) {
    if (metadata == NULL) {
        return DZ_RESULT_INVALID_ARGUMENT;
    } else if (metadata->state == DZ_BLOCK_STATE_BAD) {
        return DZ_RESULT_INVALID_STATE;
    } else {
        metadata->state = DZ_BLOCK_STATE_ACTIVE;

        memset(metadata->pageStateMap,
               DZ_PAGE_STATE_VALID,
               metadata->pageCount);

        return DZ_RESULT_OK;
    }
}

/* Marks a block as bad. */
dzResult dzBlockMarkAsBad(dzBlockMetadata *metadata) {
    if (metadata == NULL) {
        return DZ_RESULT_INVALID_ARGUMENT;
    } else if (metadata->state == DZ_BLOCK_STATE_FREE) {
        return DZ_RESULT_INVALID_ARGUMENT;
    } else {
        metadata->nextPageId = DZ_PAGE_INVALID_ID;
        metadata->state = DZ_BLOCK_STATE_BAD;

        memset(metadata->pageStateMap, DZ_PAGE_STATE_BAD, metadata->pageCount);

        return DZ_RESULT_OK;
    }
}

/* Marks a block as free. */
dzResult dzBlockMarkAsFree(dzBlockMetadata *metadata, dzF64 *eraseLatency) {
    if (metadata == NULL || eraseLatency == NULL)
        return DZ_RESULT_INVALID_ARGUMENT;
    else if (metadata->state == DZ_BLOCK_STATE_BAD
             || metadata->state == DZ_BLOCK_STATE_FREE) {
        return DZ_RESULT_INVALID_STATE;
    } else {
        metadata->nextPageId = 0U;
        metadata->state = DZ_BLOCK_STATE_FREE;

        metadata->totalEraseCount++;

        memset(metadata->pageStateMap,
               DZ_PAGE_STATE_FREE,
               metadata->pageCount);

        *eraseLatency =
            dzUtilsGaussian(eraseLatencyTable[metadata->cellType],
                            DZ_BLOCK_ERASE_LATENCY_STDDEV_RATIO
                                * eraseLatencyTable[metadata->cellType]);

        return DZ_RESULT_OK;
    }
}

/* Marks a block as reserved. */
dzResult dzBlockMarkAsReserved(dzBlockMetadata *metadata) {
    if (metadata == NULL) return DZ_RESULT_INVALID_ARGUMENT;

    metadata->state = DZ_BLOCK_STATE_RESERVED;

    // TODO: ...

    return DZ_RESULT_OK;
}

/* Marks a block as unknown. */
dzResult dzBlockMarkAsUnknown(dzBlockMetadata *metadata) {
    if (metadata == NULL) return DZ_RESULT_INVALID_ARGUMENT;

    metadata->state = DZ_BLOCK_STATE_UNKNOWN;

    memset(metadata->pageStateMap, DZ_PAGE_STATE_UNKNOWN, metadata->pageCount);

    return DZ_RESULT_OK;
}

/* Updates the state of the given page within a block's page state map. */
dzResult dzBlockUpdatePageStateMap(dzBlockMetadata *metadata,
                                   dzPPA ppa,
                                   dzPageState pageState) {
    if (metadata == NULL || metadata->nextPageId == DZ_PAGE_INVALID_ID
        || metadata->pageStateMap == NULL
        || !dzUtilsPBAEquals(metadata->pba, ppa)
        || ppa.pageId != metadata->nextPageId)
        return DZ_RESULT_INVALID_ARGUMENT;

    metadata->pageStateMap[metadata->nextPageId] = (dzByte) pageState;

    return DZ_RESULT_OK;
}
