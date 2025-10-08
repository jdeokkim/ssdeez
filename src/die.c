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
    dzDieStatistics stats;
    dzDieMetadata metadata;
    dzBlockMetadata *blockMetadata;
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

/* Returns the memory address of the `pageId`-th page in `die`. */
static DZ_API_INLINE dzByte *dzDiePageIdToPtr_(const dzDie *die, dzU64 pageId);

/* Returns the `die`-local page identifier corresponding to `pagePtr`. */
static DZ_API_INLINE dzU64 dzDiePagePtrToId_(const dzDie *die,
                                             const dzByte *pagePtr);

/* Public Functions =======================================================> */

/* Creates a die with the given `config`. */
dzDie *dzDieCreate(dzDieConfig config) {
    dzDie *die = malloc(sizeof *die);

    if (die == NULL) return die;

    die->config = config;

    die->stats = (dzDieStatistics) { .totalProgramLatency = 0.0,
                                     .totalProgramCount = 0U,
                                     .totalReadLatency = 0.0,
                                     .totalReadCount = 0U };

    {
        die->metadata.blockCountPerDie = config.planeCountPerDie
                                         * config.blockCountPerPlane;

        die->metadata.pageCountPerDie = die->metadata.blockCountPerDie
                                        * config.layerCountPerBlock;

        die->metadata.physicalPageSize = config.pageSizeInBytes
                                         + dzPageGetMetadataSize();
    }

    {
        dzUSize blockMetadataSize = dzBlockGetMetadataSize();

        die->blockMetadata = malloc(die->metadata.blockCountPerDie
                                    * blockMetadataSize);

        for (dzU64 i = 0U; i < die->metadata.blockCountPerDie; i++) {
            dzByte *blockMetadataPtr = ((dzByte *) die->blockMetadata)
                                       + (i * blockMetadataSize);

            dzBlockConfig blockConfig = { .blockId = i };

            if (!dzBlockInitMetadata(blockMetadataPtr, blockConfig)) {
                dzDieRelease(die);

                return NULL;
            }
        }
    }

    if ((die->buffer = dzDieCreateBuffer(config, die->metadata)) == NULL) {
        dzDieRelease(die);

        return NULL;
    }

    return die;
}

/* Releases the memory allocated for `die`. */
void dzDieRelease(dzDie *die) {
    if (die == NULL) return;

    free(die->blockMetadata), free(die->buffer), free(die);
}

/* Returns the total number of pages in `die`. */
dzU64 dzDieGetPageCount(const dzDie *die) {
    return (die != NULL) ? die->metadata.pageCountPerDie : 0U;
}

/* Writes `srcBuffer` to the `pageId`-th page in `die`. */
bool dzDieProgramPage(dzDie *die, dzU64 pageId, const void *srcBuffer) {
    dzByte *pagePtr = dzDiePageIdToPtr_(die, pageId);

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

/* Reads data from the `pageId`-th page in `die`, copying it to `dstBuffer`. */
bool dzDieReadPage(dzDie *die, dzU64 pageId, void *dstBuffer) {
    dzByte *pagePtr = dzDiePageIdToPtr_(die, pageId);

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

/* Returns the memory address of the `pageId`-th page in `die`. */
dzByte *dzDiePageIdToPtr(const dzDie *die, dzU64 pageId) {
    return dzDiePageIdToPtr_(die, pageId);
}

/* Returns the `die`-local page identifier corresponding to `pagePtr`. */
dzU64 dzDiePagePtrToId(const dzDie *die, const dzByte *pagePtr) {
    return dzDiePagePtrToId_(die, pagePtr);
}

/* Private Functions ======================================================> */

/* Creates a die buffer with the given `config` and `metadata`. */
static dzByte *dzDieCreateBuffer(dzDieConfig config, dzDieMetadata metadata) {
    dzByte *result = NULL;

    dzU64 bufferSize = metadata.pageCountPerDie * metadata.physicalPageSize;

    if ((result = malloc(bufferSize * sizeof *result)) == NULL) return result;

    // NOTE: Simulating the 'erased' state by setting all bits to `1`
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

/* Returns the `die`-local page identifier corresponding to `pagePtr`. */
static DZ_API_INLINE dzU64 dzDiePagePtrToId_(const dzDie *die,
                                             const dzByte *pagePtr) {
    if (!dzDieIsValidPagePtr_(die, pagePtr)) return DZ_PAGE_INVALID_ID;

    dzU64 ptrOffset = (dzU64) (pagePtr - die->buffer);

    return ptrOffset / die->metadata.physicalPageSize;
}

/* Returns the memory address of the `pageId`-th page in `die`. */
static DZ_API_INLINE dzByte *dzDiePageIdToPtr_(const dzDie *die,
                                               dzU64 pageId) {
    if (die == NULL || die->buffer == NULL
        || pageId >= die->metadata.pageCountPerDie)
        return NULL;

    return die->buffer + (pageId * die->metadata.physicalPageSize);
}
