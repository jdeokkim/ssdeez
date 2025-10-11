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

/* A constant that represents an invalid block id. */
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
