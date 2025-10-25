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

#include "greatest.h"
#include "ssdeez.h"

/* Macros =================================================================> */

// TODO: ...

/* Constants ==============================================================> */

// NOTE: Based on the specs provided in the Samsung K9F2G08U0M datasheet
static const dzDieConfig dieConfig = { .dieId = 0U,
                                       .cellType = DZ_CELL_TYPE_SLC,
                                       .badBlockRatio = 0.0,
                                       .planeCountPerDie = 1U,
                                       .blockCountPerPlane = 2048U,
                                       .pageCountPerBlock = 64U,
                                       .pageSizeInBytes = 2048U };

/* Private Variables ======================================================> */

static dzDie *die = NULL;

/* Private Function Prototypes ============================================> */

static void dzTestSetupCb(void *ctx);
static void dzTestTeardownCb(void *ctx);

TEST dzTestPageOps(void);
TEST dzTestBlockOps(void);

/* Public Functions =======================================================> */

SUITE(dzTestDieOps) {
    SET_SETUP(dzTestSetupCb, NULL);
    SET_TEARDOWN(dzTestTeardownCb, NULL);

    RUN_TEST(dzTestPageOps);
    RUN_TEST(dzTestBlockOps);
}

/* Private Functions ======================================================> */

static void dzTestSetupCb(void *ctx) {
    DZ_API_UNUSED_VARIABLE(ctx);

    die = dzDieCreate(dieConfig);
}

static void dzTestTeardownCb(void *ctx) {
    DZ_API_UNUSED_VARIABLE(ctx);

    dzDieRelease(die), die = NULL;
}

/* ========================================================================> */

TEST dzTestPageOps(void) {
    ASSERT_NEQ(NULL, die);

    // clang-format off

    const dzByte srcData[] = { 
        0x12, 0x34, 0x56, 0x78, 0x91, 0x23, 0x45, 0x67,
        0x89, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23
    };

    // clang-format on

    dzSizedBuffer srcBuffer = { .ptr = (dzByte *) srcData,
                                .size = sizeof srcData };

    ASSERT_LTE(srcBuffer.size, dieConfig.pageSizeInBytes);

    for (dzPPA ppa = dzDieGetFirstPPA(die); ppa.pageId != DZ_PAGE_INVALID_ID;
         ppa = dzDieGetNextPPA(die, ppa)) {
        // if (dzDieGetPageState(die, ppa) == DZ_PAGE_STATE_BAD) continue;

        ASSERT_EQ(true, dzDieProgramPage(die, ppa, srcBuffer));

        // NOTE: This page is already programmed!
        ASSERT_EQ(false, dzDieProgramPage(die, ppa, srcBuffer));
    }

    dzByte dstData[sizeof srcData];

    dzSizedBuffer dstBuffer = { .ptr = (dzByte *) dstData,
                                .size = sizeof dstData };

    for (dzPPA ppa = dzDieGetFirstPPA(die); ppa.pageId != DZ_PAGE_INVALID_ID;
         ppa = dzDieGetNextPPA(die, ppa)) {
        // if (dzDieGetPageState(die, ppa) == DZ_PAGE_STATE_BAD) continue;

        // NOTE: Making sure `dzDieReadPage()` is doing its job well
        memset(dstBuffer.ptr, 0xFF, dstBuffer.size);

        ASSERT_EQ(true, dzDieReadPage(die, ppa, dstBuffer));

        ASSERT_MEM_EQ(srcBuffer.ptr, dstBuffer.ptr, srcBuffer.size);
    }

    PASS();
}

TEST dzTestBlockOps(void) {
    ASSERT_NEQ(NULL, die);

    // clang-format off

    const dzByte srcData[] = { 
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
    };

    // clang-format on

    dzSizedBuffer srcBuffer = { .ptr = (dzByte *) srcData,
                                .size = sizeof srcData };

    ASSERT_LTE(srcBuffer.size, dieConfig.pageSizeInBytes);

    {
        for (dzPPA ppa = dzDieGetFirstPPA(die);
             ppa.pageId != DZ_PAGE_INVALID_ID;
             ppa = dzDieGetNextPPA(die, ppa))
            (void) dzDieProgramPage(die, ppa, srcBuffer);
    }

    for (dzPPA pba = dzDieGetFirstPBA(die); pba.blockId != DZ_PAGE_INVALID_ID;
         pba = dzDieGetNextPBA(die, pba)) {
        // if (dzDieGetBlockState(die, pba) == DZ_BLOCK_STATE_BAD) continue;

        ASSERT_EQ(DZ_BLOCK_STATE_ACTIVE, dzDieGetBlockState(die, pba));

        ASSERT_EQ(true, dzDieEraseBlock(die, pba));

        ASSERT_EQ(DZ_BLOCK_STATE_FREE, dzDieGetBlockState(die, pba));

        // NOTE: Free blocks should not be erased again
        ASSERT_EQ(false, dzDieEraseBlock(die, pba));
    }

    {
        dzByte dstData[sizeof srcData];

        dzSizedBuffer dstBuffer = { .ptr = (dzByte *) dstData,
                                    .size = sizeof dstData };

        for (dzPPA ppa = dzDieGetFirstPPA(die);
             ppa.pageId != DZ_PAGE_INVALID_ID;
             ppa = dzDieGetNextPPA(die, ppa)) {
            // if (dzDieGetPageState(die, ppa) == DZ_PAGE_STATE_BAD) continue;

            ASSERT_EQ(true, dzDieReadPage(die, ppa, dstBuffer));

            // NOTE: Check if all pages have been initialized to `0xFF`
            for (dzU64 k = 0; k < sizeof dstBuffer; k++)
                ASSERT_EQ((dzByte) 0xFF, dstBuffer.ptr[k]);
        }
    }

    PASS();
}
