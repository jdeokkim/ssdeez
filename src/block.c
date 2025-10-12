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

/* A structure that represents the metadata of a NAND flash block. */
struct dzBlockMetadata_ {
    dzU64 blockId;
    dzU64 totalEraseCount;
    // dzF64 lastEraseTime;
    dzCellType cellType;
    dzBlockState state;
    // TODO: ...
};

/* Typedefs ===============================================================> */

// TODO: ...

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

/* Initializes a block metadata within the given `blockMetadata`. */
bool dzBlockInitMetadata(dzBlockMetadata *blockMetadata,
                         dzBlockConfig config) {
    if (blockMetadata == NULL) return false;

    {
        blockMetadata->blockId = config.blockId;

        blockMetadata->totalEraseCount = 0U;
        // blockMetadata->lastEraseTime = 0.0;

        blockMetadata->cellType = config.cellType;

        blockMetadata->state = DZ_BLOCK_STATE_FREE;
    }

    return true;
}

/* Returns the size of `dzBlockMetadata`. */
dzUSize dzBlockGetMetadataSize(void) {
    return sizeof(dzBlockMetadata);
}

/* Returns the current state of a block. */
dzBlockState dzBlockGetState(dzBlockMetadata *blockMetadata) {
    return (blockMetadata != NULL) ? blockMetadata->state
                                   : DZ_BLOCK_STATE_UNKNOWN;
}

/* Marks a block as bad. */
bool dzBlockMarkAsBad(dzBlockMetadata *blockMetadata) {
    if (blockMetadata == NULL) return false;

    blockMetadata->state = DZ_BLOCK_STATE_BAD;

    return true;
}

/* Marks a block as free. */
bool dzBlockMarkAsFree(dzBlockMetadata *blockMetadata, dzF64 *eraseLatency) {
    if (blockMetadata == NULL || eraseLatency == NULL) return false;

    blockMetadata->state = DZ_BLOCK_STATE_FREE;

    *eraseLatency =
        dzUtilsGaussian(eraseLatencyTable[blockMetadata->cellType],
                        DZ_BLOCK_ERASE_LATENCY_STDDEV_RATIO
                            * eraseLatencyTable[blockMetadata->cellType]);

    return true;
}

/* Marks a block as valid. */
bool dzBlockMarkAsValid(dzBlockMetadata *blockMetadata) {
    if (blockMetadata == NULL) return false;

    blockMetadata->state = DZ_BLOCK_STATE_VALID;

    return true;
}
