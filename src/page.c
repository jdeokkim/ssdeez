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
    dzPageStatus status;
    dzU32 peCycleCount;
    // TODO: ...
};

/* Private Function Prototypes ============================================> */

// TODO: ...

/* Constants ==============================================================> */

/* Baseline numbers of P/E cycles per page, for each cell type. */
static const dzU32 basePeCycleCounts[DZ_CELL_TYPE_COUNT_] = {
    [DZ_CELL_TYPE_SLC] = 85000U,
    [DZ_CELL_TYPE_MLC] = 19000U,
    [DZ_CELL_TYPE_TLC] = 2250U,
    [DZ_CELL_TYPE_QLC] = 750U
};

/* Baseline 'program' latencies for each cell type, in miliseconds. */
/*
static const dzF64 baseProgramLatencies[DZ_CELL_TYPE_COUNT_] = {
    [DZ_CELL_TYPE_SLC] = 1.0f,
    [DZ_CELL_TYPE_MLC] = 2.5f,
    [DZ_CELL_TYPE_TLC] = 3.75f,
    [DZ_CELL_TYPE_QLC] = 5.5f
};
*/

/* Baseline 'read' latencies for each cell type, in miliseconds. */
/*
static const dzF64 baseReadLatencies[DZ_CELL_TYPE_COUNT_] = {
    [DZ_CELL_TYPE_SLC] = 0.015f,
    [DZ_CELL_TYPE_MLC] = 0.035f,
    [DZ_CELL_TYPE_TLC] = 0.060f,
    [DZ_CELL_TYPE_QLC] = 0.095f
};
*/

/* Private Variables ======================================================> */

// TODO: ...

/* Public Functions =======================================================> */

/* Creates an array of page metadata objects based on the given `config`. */
dzPageMetadata *dzPageCreateMetadata(dzDieConfig config) {
    if (config.cellType <= DZ_CELL_TYPE_UNKNOWN
        || config.cellType >= DZ_CELL_TYPE_COUNT_)
        return NULL;

    // clang-format off

    dzU64 pageCountPerDie = config.planeCountPerDie 
        * config.blockCountPerPlane
        * config.layerCountPerBlock;

    // clang-format on

    dzPageMetadata *pageMetadata = malloc(pageCountPerDie
                                          * sizeof *pageMetadata);

    {
        dzU64 centerPageIndex = pageCountPerDie >> 1;

        // NOTE: Pages in the top and bottom layers should have lower endurance
        dzBool shouldApplyPenalty = pageCountPerDie > 2;

        for (dzU64 i = 0U; i < pageCountPerDie; i++) {
            pageMetadata[i].status = DZ_PAGE_STATUS_FREE;

            pageMetadata[i].peCycleCount =
                dzUtilsGaussian(basePeCycleCounts[config.cellType],
                                DZ_PAGE_PE_CYCLE_COUNT_STDDEV);

            if (shouldApplyPenalty) {
                dzF32 distanceFromCenter = (i > centerPageIndex)
                                               ? (centerPageIndex - i)
                                               : (i - centerPageIndex);

                dzF32 penaltyScale = distanceFromCenter / centerPageIndex;

                // clang-format off

                pageMetadata[i].peCycleCount *= 1.0f - \
                    (penaltyScale * DZ_PAGE_PE_CYCLE_COUNT_MAX_PENALTY);

                // clang-format on
            }
        }
    }

    return pageMetadata;
}

/* Releases the memory allocated for `pageMetadata`. */
void dzPageReleaseMetadata(dzPageMetadata *pageMetadata) {
    free(pageMetadata);
}

/* Marks the `pageIndex`-th page as valid. */
bool dzPageMarkAsValid(dzPageMetadata *pageMetadata, dzU64 pageIndex) {
    if (pageMetadata == NULL
        || pageMetadata[pageIndex].status != DZ_PAGE_STATUS_FREE
        || pageMetadata[pageIndex].peCycleCount == 0U)
        return false;

    pageMetadata[pageIndex].status = DZ_PAGE_STATUS_VALID;

    // TODO: P/E Latency

    return true;
}

/* Marks the `pageIndex`-th page as free. */
bool dzPageMarkAsFree(dzPageMetadata *pageMetadata, dzU64 pageIndex) {
    if (pageMetadata == NULL
        || pageMetadata[pageIndex].status == DZ_PAGE_STATUS_BAD
        || pageMetadata[pageIndex].status == DZ_PAGE_STATUS_FREE
        || pageMetadata[pageIndex].peCycleCount == 0U)
        return false;

    pageMetadata[pageIndex].peCycleCount--;

    pageMetadata[pageIndex].status = (pageMetadata[pageIndex].peCycleCount > 0)
                                         ? DZ_PAGE_STATUS_FREE
                                         : DZ_PAGE_STATUS_BAD;

    // TODO: P/E Latency

    return true;
}

/* Private Functions ======================================================> */

// TODO: ...
