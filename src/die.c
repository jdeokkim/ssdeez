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

#include <math.h>

#include "ssdeez.h"

/* Macros =================================================================> */

#define dzDiePageForEach(dieBuffer, dieMetadata, ptrIdentifier) \
    for (dzByte *ptrIdentifier = (dieBuffer),                   \
                *ptrIdentifier##__LINE__ =                      \
                    (dieBuffer)                                 \
                    + ((dieMetadata).pageCountPerDie            \
                       * (dieMetadata).physicalPageSize);       \
         ptrIdentifier < ptrIdentifier##__LINE__;               \
         ptrIdentifier += (dieMetadata).physicalPageSize)

/* Typedefs ===============================================================> */

/* A structure that represents the metadata of a NAND flash die. */
struct dzDieMetadata_ {
    dzU64 pageCountPerDie;
    dzU64 physicalPageSize;
    // TODO: ...
};

/* A structure that represents a group of NAND flash planes. */
struct dzDie_ {
    dzDieConfig config;
    dzDieMetadata metadata;
    dzByte *buffer;
    // TODO: ...
};

/* Private Variables ======================================================> */

// TODO: ...

/* Private Function Prototypes ============================================> */

/* Creates a die buffer with the given `config` and `metadata`. */
static dzByte *dzDieCreateBuffer(dzDieConfig config, dzDieMetadata metadata);

/* Returns `true` if `pagePtr` is pointing to a valid page in a die. */
static DZ_API_INLINE bool dzDieIsValidPage(const dzDie *die,
                                           const dzByte *pagePtr);

/* Public Functions =======================================================> */

/* Creates a die with the given `config`. */
dzDie *dzDieCreate(dzDieConfig config) {
    dzDie *die = malloc(sizeof *die);

    if (die == NULL) return die;

    die->config = config;

    // clang-format off

    die->metadata = (dzDieMetadata) {
        .pageCountPerDie = config.planeCountPerDie 
                             * config.blockCountPerPlane
                             * config.layerCountPerBlock,
        .physicalPageSize = config.pageSizeInBytes 
                             + dzPageGetMetadataSize()
    };

    // clang-format on

    if ((die->buffer = dzDieCreateBuffer(config, die->metadata)) == NULL) {
        dzDieRelease(die);

        return NULL;
    }

    return die;
}

/* Releases the memory allocated for `die`. */
void dzDieRelease(dzDie *die) {
    if (die == NULL) return;

    free(die->buffer), free(die);
}

/* Converts `pagePtr` to a physical page number. */
dzU64 dzDiePtrToPPN(const dzDie *die, const dzByte *pagePtr) {
    if (!dzDieIsValidPage(die, pagePtr)) return DZ_PAGE_INVALID_PPN;

    dzISize ptrOffset = pagePtr - die->buffer;

    return (dzU64) (ptrOffset / die->config.pageSizeInBytes);
}

/* Converts `ppn` to a physical page address. */
dzByte *dzDiePPNToPtr(const dzDie *die, dzU64 ppn) {
    if (die == NULL || die->buffer == NULL
        || ppn >= die->metadata.pageCountPerDie)
        return NULL;

    return die->buffer + (ppn * die->metadata.physicalPageSize);
}

/* Private Functions ======================================================> */

/* Creates a die buffer with the given `config` and `metadata`. */
static dzByte *dzDieCreateBuffer(dzDieConfig config, dzDieMetadata metadata) {
    dzByte *result = NULL;

    dzU64 bufferSize = metadata.pageCountPerDie * metadata.physicalPageSize;

    if ((result = malloc(bufferSize * sizeof *result)) == NULL) return result;

    dzU64 pageIndex = 0U, centerPageIndex = metadata.pageCountPerDie >> 1;

    dzDiePageForEach(result, metadata, pagePtr) {
        dzF64 peCycleCountPenalty = 1.0;

        // NOTE: Pages in the top and bottom layers should have lower endurance
        if (metadata.pageCountPerDie > 2) {
            dzU64 distanceFromCenter = (pageIndex > centerPageIndex)
                                           ? (pageIndex - centerPageIndex)
                                           : (centerPageIndex - pageIndex);

            dzF64 penaltyScale = ((dzF64) distanceFromCenter
                                  / (dzF64) centerPageIndex);

            peCycleCountPenalty -= (penaltyScale
                                    * DZ_PAGE_PE_CYCLE_COUNT_MAX_PENALTY);
        }

        dzPageConfig pageConfig = {
            .cellType = config.cellType,
            .pageSizeInBytes = config.pageSizeInBytes,
            .peCycleCountPenalty = peCycleCountPenalty
        };

        if (!dzPageInitMetadata(pagePtr, pageConfig)) {
            free(result);

            return NULL;
        }

        pageIndex++;
    }

    return result;
}

/* Returns `true` if `pagePtr` is pointing to a valid page in a die. */
static DZ_API_INLINE bool dzDieIsValidPage(const dzDie *die,
                                           const dzByte *pagePtr) {
    if (die == NULL || pagePtr == NULL) return false;

    const dzByte *firstPagePtr = die->buffer;
    const dzByte *lastPagePtr = firstPagePtr
                                + (die->metadata.pageCountPerDie
                                   * die->metadata.physicalPageSize);

    return (pagePtr >= firstPagePtr && pagePtr < lastPagePtr);
}