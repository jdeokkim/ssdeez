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

#include <string.h>

#include "ssdeez.h"

/* Macros =================================================================> */

// TODO: ...

/* Typedefs ===============================================================> */

// TODO: ...

/* Constants ==============================================================> */

/* Minimum number of bytes required to create a valid ONFI parameter page. */
static const dzUSize DZ_ONFI_PARAMETER_PAGE_MIN_SIZE = 768U;

/* ONFI revision number which includes support for ONFI version 1.0. */
static const dzU16 DZ_ONFI_REVISION_NUMBER = 0x0002U;

/* Private Variables ======================================================> */

/* Signature bytes for a valid ONFI parameter page. */
static dzByte DZ_ONFI_PARAMETER_PAGE_SIGNATURE[] = { 'O', 'N', 'F', 'I' };

// clang-format off

/* Device Manufacturer, padded to 12 bytes with spaces. */
static dzByte DZ_ONFI_DEVICE_MANUFACTURER[] = { 
    'S', 'S', 'D', 'e', 'e', 'z', 
    ' ', ' ', ' ', ' ', ' ', ' '
};

/* Device Model, padded to 20 bytes with spaces. */
static dzByte DZ_ONFI_DEVICE_MODEL[] = {
    'N', '-', 'U', 'T', 'S', 
    ' ', ' ', ' ', ' ', ' ', 
    ' ', ' ', ' ', ' ', ' ', 
    ' ', ' ', ' ', ' ', ' '
};

// clang-format on

/* Private Function Prototypes ============================================> */

/* Writes 1 byte to `dst->ptr` and advances `dst->offset`. */
DZ_API_STATIC_INLINE void dzOnfiWriteByte(dzByteStream *dst, dzByte value);

/* Writes multiple bytes to `dst->ptr` and advances `dst->offset`. */
DZ_API_STATIC_INLINE void dzOnfiWriteBytes(dzByteStream *dst,
                                           dzByteArray value);

/* Writes 2 bytes to `dst->ptr` and advances `dst->offset`. */
DZ_API_STATIC_INLINE void dzOnfiWriteWord(dzByteStream *dst, dzU16 value);

/* Writes 4 bytes to `dst->ptr` and advances `dst->offset`. */
DZ_API_STATIC_INLINE void dzOnfiWriteDword(dzByteStream *dst, dzU32 value);

/* Writes `size` zeroes to `dst->ptr` and advances `dst->offset`. */
DZ_API_STATIC_INLINE void dzOnfiWriteZeroes(dzByteStream *dst, dzU64 size);

/* ========================================================================> */

/* 
    Writes the "Revision Information and Features" section of a parameter page 
    to `dst->ptr` and advances `dst->offset`.
*/
DZ_API_STATIC_INLINE void dzOnfiWriteRIFSection(dzByteStream *dst);

/* 
    Writes the "Manufacturer Information" section of a parameter page 
    to `dst->ptr` and advances `dst->offset`.
*/
DZ_API_STATIC_INLINE void dzOnfiWriteMISection(dzByteStream *dst);

/* 
    Writes the "Memory Organization" section of a parameter page 
    to `dst->ptr` and advances `dst->offset`.
*/
DZ_API_STATIC_INLINE void dzOnfiWriteMOSection(const dzDie *die,
                                               dzByteStream *dst);

/* Public Functions =======================================================> */

/* 
    Creates a new ONFI parameter page based on `die`'s configuration, 
    and writes the contents of it to `dst.ptr`.
*/
dzResult dzOnfiCreateParameterPage(const dzDie *die, dzByteArray dst) {
    if (dst.ptr == NULL || dst.size < DZ_ONFI_PARAMETER_PAGE_MIN_SIZE)
        return DZ_RESULT_INVALID_ARGUMENT;

    dzByteStream dstStream = { .ptr = dst.ptr, .size = dst.size };

    dzOnfiWriteRIFSection(&dstStream);
    dzOnfiWriteMISection(&dstStream);

    dzOnfiWriteMOSection(die, &dstStream);

    // TODO: ...

    return DZ_RESULT_OK;
}

/* Private Functions ======================================================> */

/* Writes 1 byte to `dst->ptr` and advances `dst->offset`. */
DZ_API_STATIC_INLINE void dzOnfiWriteByte(dzByteStream *dst, dzByte value) {
    if (dst == NULL || dst->ptr == NULL || dst->offset >= dst->size) return;

    dst->ptr[dst->offset++] = value;
}

/* Writes multiple bytes to `dst->ptr` and advances `dst->offset`. */
DZ_API_STATIC_INLINE void dzOnfiWriteBytes(dzByteStream *dst,
                                           dzByteArray value) {
    if (dst == NULL || dst->ptr == NULL || dst->offset >= dst->size
        || (dst->size - dst->offset) < value.size || value.ptr == NULL)
        return;

    (void) memcpy(dst->ptr + dst->offset, value.ptr, value.size);

    dst->offset += value.size;
}

/* Writes 2 bytes to `dst->ptr` and advances `dst->offset`. */
DZ_API_STATIC_INLINE void dzOnfiWriteWord(dzByteStream *dst, dzU16 value) {
    if (dst == NULL || dst->ptr == NULL || dst->offset >= dst->size
        || (dst->size - dst->offset) < 2U)
        return;

    dst->ptr[dst->offset] = value & 0xFFU;
    dst->ptr[dst->offset + 1U] = (value >> 8U) & 0xFFU;

    dst->offset += 2U;
}

/* Writes 4 bytes to `dst->ptr` and advances `dst->offset`. */
DZ_API_STATIC_INLINE void dzOnfiWriteDword(dzByteStream *dst, dzU32 value) {
    if (dst == NULL || dst->ptr == NULL || dst->offset >= dst->size
        || (dst->size - dst->offset) < 4U)
        return;

    dst->ptr[dst->offset] = value & 0xFFU;
    dst->ptr[dst->offset + 1U] = (value >> 8U) & 0xFFU;
    dst->ptr[dst->offset + 2U] = (value >> 16U) & 0xFFU;
    dst->ptr[dst->offset + 3U] = (value >> 24U) & 0xFFU;

    dst->offset += 4U;
}

/* Writes `size` zeroes to `dst->ptr` and advances `dst->offset`. */
DZ_API_STATIC_INLINE void dzOnfiWriteZeroes(dzByteStream *dst, dzU64 size) {
    if (dst == NULL || dst->ptr == NULL || dst->offset >= dst->size
        || (dst->size - dst->offset) < size)
        return;

    (void) memset(dst->ptr + dst->offset, 0, size);

    dst->offset += size;
}

/* ========================================================================> */

/* 
    Writes the "Revision Information and Features" section 
    of an ONFI parameter page to `dst->ptr` and advances `dst->offset`.
*/
DZ_API_STATIC_INLINE void dzOnfiWriteRIFSection(dzByteStream *dst) {
    /* "Parameter Page Signature" */
    dzOnfiWriteBytes(dst,
                     (dzByteArray) {
                         .ptr = DZ_ONFI_PARAMETER_PAGE_SIGNATURE,
                         .size = sizeof DZ_ONFI_PARAMETER_PAGE_SIGNATURE });

    /* "Revision Number" */
    dzOnfiWriteWord(dst, DZ_ONFI_REVISION_NUMBER);

    /* "Features Supported" */

    // TODO: More features?
    dzOnfiWriteWord(dst, 0x0000U);

    /* "Optional Commands Supported" */

    // TODO: More optional commands?
    dzOnfiWriteWord(dst, 0x0000U);

    /* "Reserved" */
    dzOnfiWriteZeroes(dst, 22U);
}

/* 
    Writes the "Manufacturer Information" section 
    of an ONFI parameter page to `dst->ptr` and advances `dst->offset`.
*/
DZ_API_STATIC_INLINE void dzOnfiWriteMISection(dzByteStream *dst) {
    /* "Device Manufacturer" */

    dzOnfiWriteBytes(dst,
                     (dzByteArray) {
                         .ptr = DZ_ONFI_DEVICE_MANUFACTURER,
                         .size = sizeof DZ_ONFI_DEVICE_MANUFACTURER });

    /* "Device Model" */
    dzOnfiWriteBytes(dst,
                     (dzByteArray) { .ptr = DZ_ONFI_DEVICE_MODEL,
                                     .size = sizeof DZ_ONFI_DEVICE_MODEL });

    /* "JEDEC Manufacturer ID" */
    dzOnfiWriteByte(dst, 0x00U);

    /* "Date Code" */

    // NOTE: Week 46, 2025
    dzOnfiWriteWord(dst, 0x2E19U);

    /* "Reserved" */
    dzOnfiWriteZeroes(dst, 13U);
}

/* 
    Writes the "Memory Organization" section of a parameter page 
    to `dst->ptr` and advances `dst->offset`.
*/
DZ_API_STATIC_INLINE void dzOnfiWriteMOSection(const dzDie *die,
                                               dzByteStream *dst) {
    dzDieConfig dieConfig = dzDieGetConfig(die);

    /* "Number of Data Bytes per Page" */
    dzOnfiWriteDword(dst, dieConfig.pageSizeInBytes);

    /* "Number of Spare Bytes per Page" */
    dzOnfiWriteWord(dst, (dzU16) dzPageGetMetadataSize());

    /* "Number of Data Bytes per Partial Page" */
    dzOnfiWriteDword(dst, 0x00000000U);

    /* "Number of Spare Bytes per Partial Page" */
    dzOnfiWriteWord(dst, 0x0000U);

    /* "Number of Pages per Block" */
    dzOnfiWriteDword(dst, dieConfig.pageCountPerBlock);

    /* "Number of Blocks per Logical Unit (LUN)" */

    // NOTE: Assuming each logical unit consists of only one die
    dzOnfiWriteDword(dst,
                     dieConfig.planeCountPerDie
                         * dieConfig.blockCountPerPlane);

    /* "Number of Logical Units (LUNs)" */
    dzOnfiWriteByte(dst, 0x01U);

    /* "Number of Address Cycles" */

    // TODO: How do I figure out an appropriate value???
    dzOnfiWriteByte(dst, 0x00U);

    /* "Number of Bits per Cell" */
    dzOnfiWriteByte(dst, (dzByte) dieConfig.cellType);

    /* "Bad Blocks Maximum per LUN" */
    dzOnfiWriteWord(dst,
                    (dzU16) (dieConfig.badBlockRatio
                             * (dieConfig.planeCountPerDie
                                * dieConfig.blockCountPerPlane)));

    {
        /* "Block Endurance" */

        dzU32 enduranceValue = dzDieGetMaxPeCycles(die);

        dzByte enduranceMultiplier = 0U;

        while (enduranceValue >= 100U)
            enduranceValue /= 10U, enduranceMultiplier++;

        dzOnfiWriteByte(dst, (enduranceValue & 0xFFU));
        dzOnfiWriteByte(dst, enduranceMultiplier);
    }

    /* "Guaranteed Valid Blocks at Beginning of Target" */
    dzOnfiWriteByte(dst, 0x01U);

    /* "Block Endurance for Guaranteed Valid Blocks" */
    dzOnfiWriteWord(dst, 0x0000U);

    /* "Number of Programs per Page" */
    dzOnfiWriteByte(dst, 0x01U);

    /* "Partial Programming Attributes" */
    dzOnfiWriteByte(dst, 0x00U);

    /* "Number of Bits ECC Correctability" */

    // TODO: Implement ECC?
    dzOnfiWriteByte(dst, 0x00U);

    /* "Number of Interleaved Address Bits" */
    dzOnfiWriteByte(dst, 0x00U);

    /* "Interleaved Operation Attributes" */
    dzOnfiWriteByte(dst, 0x00U);

    /* "Reserved" */
    dzOnfiWriteZeroes(dst, 13U);
}
