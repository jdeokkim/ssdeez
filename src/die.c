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

/* A structure that represents a group of NAND flash planes. */
struct dzDie_ {
    dzByte *buffer;        // The buffer used to access plane, block, etc.
    dzU32 *peCycleCounts;  // The number of P/E cycles per page
    dzDieConfig config;    // The configuration of this die
    dzU64 cellCount;       // The total number of cells in this die
    // TODO: ...
};

/* Private Function Prototypes ============================================> */

// TODO: ...

/* Constants ==============================================================> */

/* The "baseline" numbers of P/E cycles per page, for each cell type. */
static const dzU32 basePeCycleCounts[DZ_CELL_TYPE_COUNT_] = {
    [DZ_CELL_TYPE_SLC] = 85000U,
    [DZ_CELL_TYPE_MLC] = 19000U,
    [DZ_CELL_TYPE_TLC] = 2250U,
    [DZ_CELL_TYPE_QLC] = 750U
};

static const dzF32 basePeCycleCountPenalty = 0.2f;

/* Private Variables ======================================================> */

// TODO: ...

/* Public Functions =======================================================> */

/* Creates a die with the given configuration. */
dzDie *dzDieCreate(dzDieConfig config) {
    dzDie *die = malloc(sizeof *die);

    die->config = config;

    // clang-format off

    dzU64 pageCountPerDie = config.planeCountPerDie 
        * config.blockCountPerPlane
        * config.layerCountPerBlock;

    // clang-format on

    // NOTE: Allocating a byte for each cell in a page!
    die->buffer = malloc(config.pageSizeInBytes * sizeof *(die->buffer));

    {
        die->peCycleCounts = malloc(pageCountPerDie
                                    * sizeof *(die->peCycleCounts));

        dzBool hasMultiLayers = pageCountPerDie > 2;

        dzU64 centerPageIndex = pageCountPerDie >> 1;

        for (dzU64 i = 0U; i < pageCountPerDie; i++) {
            die->peCycleCounts[i] =
                dzUtilsGaussian(basePeCycleCounts[config.cellType], 128.0f);

            if (hasMultiLayers) {
                // clang-format off

                dzF32 distanceFromCenter = (i > centerPageIndex) 
                    ? (centerPageIndex - i) 
                    : (i - centerPageIndex);

                dzF32 penaltyScale = distanceFromCenter / centerPageIndex;

                /*
                    NOTE: Pages in the top and bottom layers
                          should have the lowest endurance
                */

                die->peCycleCounts[i] *= 1.0f - \
                    (penaltyScale * basePeCycleCountPenalty);

                // clang-format on
            }
        }
    }

    return die;
}

/* Releases the memory allocated for `die`. */
void dzDieRelease(dzDie *die) {
    if (die == NULL) return;

    free(die->peCycleCounts), free(die->buffer), free(die);
}

/* Private Functions ======================================================> */

// TODO: ...
