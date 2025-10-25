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

/* A structure that represents the metadata of a NAND flash plane. */
struct dzPlaneMetadata_ {
    dzByte *blockStateMap;
    dzU64 blockCount;
    dzU64 planeId;
    // TODO: ...
};

/* Constants ==============================================================> */

/* A constant that represents an invalid plane identifier. */
const dzU64 DZ_PLANE_INVALID_ID = UINT64_MAX;

/* Private Variables ======================================================> */

// TODO: ...

/* Private Function Prototypes ============================================> */

// TODO: ...

/* Public Functions =======================================================> */

/* Initializes a plane metadata within the given `metadata` region. */
bool dzPlaneInitMetadata(dzPlaneMetadata *metadata, dzPlaneConfig config) {
    if (metadata == NULL) return false;

    {
        metadata->blockStateMap = malloc(config.blockCount
                                         * sizeof *(metadata->blockStateMap));

        for (dzU64 i = 0U; i < config.blockCount; i++)
            metadata->blockStateMap[i] = DZ_BLOCK_STATE_FREE;
    }

    {
        metadata->blockCount = config.blockCount;

        metadata->planeId = config.planeId;

        // TODO: ...
    }

    return true;
}

/* De-initializes the plane `metadata`. */
void dzPlaneDeinitMetadata(dzPlaneMetadata *metadata) {
    if (metadata == NULL) return;

    free(metadata->blockStateMap);
}

/* Returns the size of `dzPlaneMetadata`. */
dzUSize dzPlaneGetMetadataSize(void) {
    return sizeof(dzPlaneMetadata);
}

/* Updates the state of the given block within a plane's block state map. */
bool dzPlaneUpdateBlockStateMap(dzPlaneMetadata *metadata,
                                dzPBA pba,
                                dzBlockState blockState) {
    if (metadata == NULL || metadata->blockStateMap == NULL
        || pba.planeId != metadata->planeId
        || pba.blockId == DZ_BLOCK_INVALID_ID)
        return false;

    metadata->blockStateMap[pba.blockId] = blockState;

    return true;
}