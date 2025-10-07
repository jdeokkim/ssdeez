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

#include <float.h>
#include <string.h>

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
    dzU64 blockCountPerDie;
    dzU64 pageCountPerDie;
    dzU64 physicalPageSize;
    // TODO: ...
};

/* A structure that represents various statistics of a NAND flash die. */
struct dzDieStatistics_ {
    dzF64 totalProgramLatency;
    dzU64 totalProgramCount;
    dzF64 totalReadLatency;
    dzU64 totalReadCount;
    // TODO: ...
};

/* A structure that represents a group of NAND flash planes. */
struct dzDie_ {
    dzDieConfig config;
    dzDieMetadata metadata;
    dzDieStatistics stats;
    dzByte *buffer;
    // TODO: ...
};

/* Private Variables ======================================================> */

// TODO: ...

/* Private Function Prototypes ============================================> */

/* Creates a die buffer with the given `config` and `metadata`. */
static dzByte *dzDieCreateBuffer(dzDieConfig config, dzDieMetadata metadata);

/* Returns `true` if `pagePtr` is pointing to a valid page in `die`. */
static DZ_API_INLINE bool dzDieIsValidPagePtr_(const dzDie *die,
                                               const dzByte *pagePtr);

/* Converts `pagePtr` to a physical page number. */
static DZ_API_INLINE dzU64 dzDiePagePtrToPPN_(const dzDie *die,
                                              const dzByte *pagePtr);

/* Converts `ppn` to a physical page address. */
static DZ_API_INLINE dzByte *dzDiePPNToPagePtr_(const dzDie *die, dzU64 ppn);

/* Public Functions =======================================================> */

/* Creates a die with the given `config`. */
dzDie *dzDieCreate(dzDieConfig config) {
    dzDie *die = malloc(sizeof *die);

    if (die == NULL) return die;

    die->config = config;

    {
        die->metadata.blockCountPerDie = config.planeCountPerDie
                                         * config.blockCountPerPlane;

        die->metadata.pageCountPerDie = die->metadata.blockCountPerDie
                                        * config.layerCountPerBlock;

        die->metadata.physicalPageSize = config.pageSizeInBytes
                                         + dzPageGetMetadataSize();
    }

    die->stats = (dzDieStatistics) { .totalProgramLatency = 0.0,
                                     .totalProgramCount = 0U,
                                     .totalReadLatency = 0.0,
                                     .totalReadCount = 0U };

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

/* Returns the total number of pages in `die`. */
dzU64 dzDieGetPageCount(const dzDie *die) {
    return (die != NULL) ? die->metadata.pageCountPerDie : 0U;
}

/* Writes `srcBuffer` to the `ppn`-th page in `die`. */
bool dzDieProgramPage(dzDie *die, dzU64 ppn, const void *srcBuffer) {
    dzByte *pagePtr = dzDiePPNToPagePtr_(die, ppn);

    if (pagePtr == NULL || srcBuffer == NULL) return false;

    {
        dzF64 programLatency = -DBL_MAX;

        // NOTE: Erase-before-Write Property!
        if (!dzPageMarkAsValid(pagePtr,
                               die->config.pageSizeInBytes,
                               &programLatency)
            || (programLatency < 0.0))
            return false;

        die->stats.totalProgramLatency += programLatency;
        die->stats.totalProgramCount++;
    }

    (void) memcpy(pagePtr, srcBuffer, die->config.pageSizeInBytes);

    return true;
}

/* Reads data from the `ppn`-th page in `die`, copying it to `dstBuffer`. */
bool dzDieReadPage(dzDie *die, dzU64 ppn, void *dstBuffer) {
    dzByte *pagePtr = dzDiePPNToPagePtr_(die, ppn);

    if (pagePtr == NULL || dstBuffer == NULL) return false;

    {
        dzF64 readLatency = -DBL_MAX;

        if (!dzPageGetReadLatency(pagePtr,
                                  die->config.pageSizeInBytes,
                                  &readLatency)
            || (readLatency < 0.0))
            return false;

        die->stats.totalReadLatency += readLatency;
        die->stats.totalReadCount++;
    }

    (void) memcpy(dstBuffer, pagePtr, die->config.pageSizeInBytes);

    return true;
}

/* Converts `pagePtr` to a physical page number. */
dzU64 dzDiePagePtrToPPN(const dzDie *die, const dzByte *pagePtr) {
    return dzDiePagePtrToPPN_(die, pagePtr);
}

/* Converts `ppn` to a physical page address. */
dzByte *dzDiePPNToPagePtr(const dzDie *die, dzU64 ppn) {
    return dzDiePPNToPagePtr_(die, ppn);
}

/* Private Functions ======================================================> */

/* Creates a die buffer with the given `config` and `metadata`. */
static dzByte *dzDieCreateBuffer(dzDieConfig config, dzDieMetadata metadata) {
    dzByte *result = NULL;

    dzU64 bufferSize = metadata.pageCountPerDie * metadata.physicalPageSize;

    if ((result = malloc(bufferSize * sizeof *result)) == NULL) return result;

    (void) memset(result, 0xFF, config.pageSizeInBytes);

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

/* Returns `true` if `pagePtr` is pointing to a valid page in `die`. */
static DZ_API_INLINE bool dzDieIsValidPagePtr_(const dzDie *die,
                                               const dzByte *pagePtr) {
    if (die == NULL || pagePtr == NULL) return false;

    const dzByte *firstPagePtr = die->buffer;
    const dzByte *lastPagePtr = firstPagePtr
                                + (die->metadata.pageCountPerDie
                                   * die->metadata.physicalPageSize);

    return (pagePtr >= firstPagePtr && pagePtr < lastPagePtr);
}

/* Converts `pagePtr` to a physical page number. */
static DZ_API_INLINE dzU64 dzDiePagePtrToPPN_(const dzDie *die,
                                              const dzByte *pagePtr) {
    if (!dzDieIsValidPagePtr_(die, pagePtr)) return DZ_PAGE_INVALID_PPN;

    dzU64 ptrOffset = (dzU64) (pagePtr - die->buffer);

    return ptrOffset / die->metadata.physicalPageSize;
}

/* Converts `ppn` to a physical page address. */
static DZ_API_INLINE dzByte *dzDiePPNToPagePtr_(const dzDie *die, dzU64 ppn) {
    if (die == NULL || die->buffer == NULL
        || ppn >= die->metadata.pageCountPerDie)
        return NULL;

    return die->buffer + (ppn * die->metadata.physicalPageSize);
}
