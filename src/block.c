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
    dzBlockConfig config;
    dzU64 totalEraseCount;
    // dzF64 lastEraseTime;
    // TODO: ...
};

/* Typedefs ===============================================================> */

// TODO: ...

/* Constants ==============================================================> */

/* A constant that represents an invalid block id. */
const dzU64 DZ_BLOCK_INVALID_ID = UINT64_MAX;

/* Private Variables ======================================================> */

// TODO: ...

/* Private Function Prototypes ============================================> */

// TODO: ...

/* Public Functions =======================================================> */

/* Initializes a block metadata within the given `blockMetadataPtr`. */
bool dzBlockInitMetadata(dzByte *blockMetadataPtr, dzBlockConfig config) {
    if (blockMetadataPtr == NULL) return false;

    dzBlockMetadata *blockMetadata = (dzBlockMetadata *) blockMetadataPtr;

    blockMetadata->config = config;

    blockMetadata->totalEraseCount = 0U;

    // blockMetadata->lastEraseTime = 0.0;

    return true;
}

/* Returns the size of `dzBlockMetadata`. */
dzUSize dzBlockGetMetadataSize(void) {
    return sizeof(dzBlockMetadata);
}
