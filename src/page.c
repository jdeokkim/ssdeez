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

/* A structure that represents the spare area of a NAND flash page. */
struct dzPageSpare_ {
    dzU16 badBlockMarker;
    // TODO: ...
};

/* Constants ==============================================================> */

// TODO: ...

/* Private Variables ======================================================> */

// TODO: ...

/* Private Function Prototypes ============================================> */

// TODO: ...

/* Public Functions =======================================================> */

/* Initializes the given `page`. */
dzResult dzPageInit(dzPage page) {
    if (page.ptr == NULL || page.size == 0U) return DZ_RESULT_INVALID_ARGUMENT;

    dzPageSpare *spareArea = (dzPageSpare *) (page.ptr + page.size);

    spareArea->badBlockMarker = 0xFFFFU;

    return DZ_RESULT_OK;
}

/* Returns the size of a NAND flash page's spare area. */
dzUSize dzPageGetSpareAreaSize(void) {
    return sizeof(dzPageSpare);
}

/* Returns `true` if `page` is defective. */
dzBool dzPageIsDefective(dzPage page) {
    if ((page.ptr == NULL) || (page.size == 0U)) return true;

    dzPageSpare *spareArea = (dzPageSpare *) (page.ptr + page.size);

    return (spareArea->badBlockMarker == 0x0000U);
}

/* Marks `page` as defective. */
dzResult dzPageMarkAsDefective(dzPage page) {
    if (page.ptr == NULL || page.size == 0U) return DZ_RESULT_INVALID_ARGUMENT;

    dzPageSpare *spareArea = (dzPageSpare *) (page.ptr + page.size);

    spareArea->badBlockMarker = 0x0000U;

    return DZ_RESULT_OK;
}

/* Private Functions ======================================================> */

// TODO: ...
