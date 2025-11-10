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

/* Private Function Prototypes ============================================> */

/* Writes a byte to `dst->ptr` and advances `dst->offset`. */
DZ_API_STATIC_INLINE void dzOnfiWriteByte(dzByteStream *dst, dzByte value);

/* Writes multiple bytes to `dst->ptr` and advances `dst->offset`. */
DZ_API_STATIC_INLINE void dzOnfiWriteBytes(dzByteStream *dst,
                                           dzByteArray value);

/* Writes two bytes to `dst->ptr` and advances `dst->offset`. */
DZ_API_STATIC_INLINE void dzOnfiWriteWord(dzByteStream *dst, dzU16 value);

/* Writes `size` zeroes to `dst->ptr` and advances `dst->offset`. */
DZ_API_STATIC_INLINE void dzOnfiWriteZeroes(dzByteStream *dst, dzU64 size);

/* ========================================================================> */

/* 
    Writes the "Revision Information and Features" section 
    of an ONFI parameter page to `dst->ptr` and advances `dst->offset`.
*/
DZ_API_STATIC_INLINE void dzOnfiWriteHeader(dzByteStream *dst);

/* Public Functions =======================================================> */

/* 
    Creates a new ONFI parameter page based on the `config`, 
    and writes the contents of it to `dst.ptr`.
*/
dzResult dzOnfiCreateParameterPage(dzDieConfig config, dzByteArray dst) {
    if (dst.ptr == NULL || dst.size < DZ_ONFI_PARAMETER_PAGE_MIN_SIZE)
        return DZ_RESULT_INVALID_ARGUMENT;

    dzByteStream dstStream = { .ptr = dst.ptr, .size = dst.size };

    // TODO: ...
    DZ_API_UNUSED_VARIABLE(config);

    dzOnfiWriteHeader(&dstStream);

    // TODO: ...

    return DZ_RESULT_OK;
}

/* Private Functions ======================================================> */

/* Writes a byte to `dst->ptr` and advances `dst->offset`. */
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

/* Writes two bytes to `dst->ptr` and advances `dst->offset`. */
DZ_API_STATIC_INLINE void dzOnfiWriteWord(dzByteStream *dst, dzU16 value) {
    if (dst == NULL || dst->ptr == NULL || dst->offset >= dst->size
        || (dst->size - dst->offset) < 2U)
        return;

    dst->ptr[dst->offset] = value & 0xFFU;
    dst->ptr[dst->offset + 1U] = (value >> 8U) & 0xFFU;

    dst->offset += 2U;
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
DZ_API_STATIC_INLINE void dzOnfiWriteHeader(dzByteStream *dst) {
    /* NOTE: "Parameter Page Signature" */

    dzOnfiWriteBytes(dst,
                     (dzByteArray) {
                         .ptr = DZ_ONFI_PARAMETER_PAGE_SIGNATURE,
                         .size = sizeof DZ_ONFI_PARAMETER_PAGE_SIGNATURE });

    /* NOTE: "Revision Number" */

    dzOnfiWriteWord(dst, DZ_ONFI_REVISION_NUMBER);

    /* NOTE: "Features Supported" */

    // TODO: More features?
    dzOnfiWriteWord(dst, 0x0000U);

    /* NOTE: "Optional Commands Supported" */

    // TODO: More optional commands?
    dzOnfiWriteWord(dst, 0x0000U);

    /* NOTE: "Reserved" */
    dzOnfiWriteZeroes(dst, 22U);
}