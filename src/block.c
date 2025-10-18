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

/* A structure that represents the metadata of a NAND flash block. */
struct dzBlockMetadata_ {
    dzU64 nextPageId;
    dzU64 lastPageId;
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
bool dzBlockInitMetadata(dzBlockMetadata *metadata, dzBlockConfig config) {
    if (metadata == NULL) return false;

    {
        metadata->nextPageId = 0U;
        metadata->lastPageId = config.lastPageId;

        metadata->totalEraseCount = 0U;
        // metadata->lastEraseTime = 0.0;

        metadata->cellType = config.cellType;

        metadata->state = DZ_BLOCK_STATE_FREE;
    }

    return true;
}

/* Returns the size of `dzBlockMetadata`. */
dzUSize dzBlockGetMetadataSize(void) {
    return sizeof(dzBlockMetadata);
}

/* Writes the next page identifier of a block to `nextPageId`. */
bool dzBlockGetNextPageId(dzBlockMetadata *metadata, dzU64 *nextPageId) {
    if (metadata == NULL || nextPageId == NULL) return false;

    *nextPageId = metadata->nextPageId;

    return true;
}

/* Returns the current state of a block. */
dzBlockState dzBlockGetState(const dzBlockMetadata *metadata) {
    return (metadata != NULL) ? metadata->state : DZ_BLOCK_STATE_UNKNOWN;
}

/* Advances the next page identifier of a block. */
bool dzBlockAdvanceNextPageId(dzBlockMetadata *metadata) {
    if (metadata == NULL) return false;

    metadata->nextPageId++;

    if (metadata->nextPageId >= metadata->lastPageId)
        metadata->nextPageId = DZ_PAGE_INVALID_ID;

    return true;
}

/* Marks a block as bad. */
bool dzBlockMarkAsBad(dzBlockMetadata *metadata) {
    if (metadata == NULL || metadata->state == DZ_BLOCK_STATE_FREE)
        return false;

    metadata->state = DZ_BLOCK_STATE_BAD;

    return true;
}

/* Marks a block as free. */
bool dzBlockMarkAsFree(dzBlockMetadata *metadata, dzF64 *eraseLatency) {
    if (metadata == NULL || eraseLatency == NULL
        || metadata->state == DZ_BLOCK_STATE_BAD
        || metadata->state == DZ_BLOCK_STATE_FREE)
        return false;

    metadata->state = DZ_BLOCK_STATE_FREE;

    *eraseLatency =
        dzUtilsGaussian(eraseLatencyTable[metadata->cellType],
                        DZ_BLOCK_ERASE_LATENCY_STDDEV_RATIO
                            * eraseLatencyTable[metadata->cellType]);

    return true;
}

/* Marks a block as valid. */
bool dzBlockMarkAsValid(dzBlockMetadata *metadata) {
    if (metadata == NULL || metadata->state == DZ_BLOCK_STATE_BAD)
        return false;

    metadata->state = DZ_BLOCK_STATE_VALID;

    return true;
}
