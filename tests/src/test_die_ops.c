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

#include "greatest.h"
#include "ssdeez.h"

/* Macros =================================================================> */

#define UNUSED(x) ((void) (x))

/* Constants ==============================================================> */

static const dzDieConfig dieConfig = { .dieId = 0U,
                                       .cellType = DZ_CELL_TYPE_MLC,
                                       .planeCountPerDie = 2U,
                                       .blockCountPerPlane = 4U,
                                       .pageCountPerBlock = 5U,
                                       .pageSizeInBytes = 16U };

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
    UNUSED(ctx);

    die = dzDieCreate(dieConfig);
}

static void dzTestTeardownCb(void *ctx) {
    UNUSED(ctx);

    dzDieRelease(die), die = NULL;
}

/* ========================================================================> */

TEST dzTestPageOps(void) {
    ASSERT_NEQ(NULL, die);

    // clang-format off

    const dzByte srcBuffer[] = { 
        0x12, 0x34, 0x56, 0x78, 0x91, 0x23, 0x45, 0x67,
        0x89, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23
    };

    ASSERT_LTE(sizeof srcBuffer, dieConfig.pageSizeInBytes);

    // clang-format on

    for (dzU64 i = 0, j = dzDieGetPageCount(die); i < j; i++) {
        ASSERT_EQ(true, dzDieProgramPage(die, i, srcBuffer));

        // NOTE: This page is already programmed!
        ASSERT_EQ(false, dzDieProgramPage(die, i, srcBuffer));
    }

    dzByte dstBuffer[sizeof srcBuffer];

    for (dzU64 i = 0, j = dzDieGetPageCount(die); i < j; i++) {
        // NOTE: Making sure `dzDieReadPage()` is doing its job well
        memset(dstBuffer, 0xFF, sizeof dstBuffer);

        ASSERT_EQ(true, dzDieReadPage(die, i, dstBuffer));

        ASSERT_MEM_EQ(srcBuffer, dstBuffer, sizeof srcBuffer);
    }

    PASS();
}

TEST dzTestBlockOps(void) {
    ASSERT_NEQ(NULL, die);

    // clang-format off

    const dzByte srcBuffer[] = { 
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
    };

    ASSERT_LTE(sizeof srcBuffer, dieConfig.pageSizeInBytes);

    // clang-format on

    {
        for (dzU64 i = 0, j = dzDieGetPageCount(die); i < j; i++)
            (void) dzDieProgramPage(die, i, srcBuffer);
    }

    for (dzU64 i = 0, j = dzDieGetBlockCount(die); i < j; i++) {
        ASSERT_EQ(DZ_BLOCK_STATE_VALID, dzDieGetBlockState(die, i));

        ASSERT_EQ(true, dzDieEraseBlock(die, i));

        ASSERT_EQ(DZ_BLOCK_STATE_FREE, dzDieGetBlockState(die, i));

        // NOTE: Free blocks should not be erased again
        ASSERT_EQ(false, dzDieEraseBlock(die, i));
    }

    {
        dzByte dstBuffer[sizeof srcBuffer];

        for (dzU64 i = 0, j = dzDieGetPageCount(die); i < j; i++) {
            ASSERT_EQ(true, dzDieReadPage(die, i, dstBuffer));

            // NOTE: Check if all pages have been initialized to `0xFF`
            for (dzU64 k = 0; k < sizeof dstBuffer; k++)
                ASSERT_EQ((dzByte) 0xFF, dstBuffer[k]);
        }
    }

    PASS();
}
