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

/* Constants ==============================================================> */

// NOTE: MT29F1G08A
static const dzDieConfig dieConfig = { .cellType = DZ_CELL_TYPE_SLC,
                                       .badBlockRatio = 0.01,
                                       .planeCountPerDie = 2U,
                                       .blockCountPerPlane = 512U,
                                       .pageCountPerBlock = 64U,
                                       .pageSizeInBytes = 2048 };

/* Private Variables ======================================================> */

// clang-format off

static dzChipConfig chipConfig = {
    .dieConfig = dieConfig, 
    .chipId = 0U, 
    .dieCount = 2U, 
    .isVerbose = false
};

// clang-format on

static dzChip *chip = NULL;

/* Private Function Prototypes ============================================> */

static void dzTestSetupCb(void *ctx);
static void dzTestTeardownCb(void *ctx);

TEST dzTestChipGetFeatures(void);
TEST dzTestChipReadID(void);
TEST dzTestChipReset(void);

/* Public Functions =======================================================> */

SUITE(dzTestChip) {
    SET_SETUP(dzTestSetupCb, NULL);
    SET_TEARDOWN(dzTestTeardownCb, NULL);

    RUN_TEST(dzTestChipGetFeatures);
    RUN_TEST(dzTestChipReadID);
    RUN_TEST(dzTestChipReset);
}

/* Private Functions ======================================================> */

static void dzTestSetupCb(void *ctx) {
    DZ_API_UNUSED_VARIABLE(ctx);

    (void) dzChipInit(&chip, chipConfig);

    dzChipSetCE(chip, 0U);
}

static void dzTestTeardownCb(void *ctx) {
    DZ_API_UNUSED_VARIABLE(ctx);

    dzChipDeinit(chip), chip = NULL;
}

/* ------------------------------------------------------------------------> */

TEST dzTestChipGetFeatures(void) {
    ASSERT_NEQ(chip, NULL);

    {
        ASSERT_EQ(dzChipGetRB(chip), 1U);

        dzChipSetCLE(chip, 1U);
        dzChipToggleWE(chip);

        dzChipWrite(chip, DZ_CHIP_CMD_GET_FEATURES, 100U);

        dzChipSetALE(chip, 1U);
        dzChipToggleWE(chip);

        // NOTE: Undefined behavior
        dzChipWrite(chip, 0x00U, 150U);
    }

    {
        dzChipSetALE(chip, 1U);
        dzChipToggleWE(chip);

        dzChipWrite(chip, 0x01U, 250U);

        dzByte data = 0xFFU;

        dzChipToggleRE(chip);

        dzChipRead(chip, &data, 350U);

        // NOTE: Timing Mode 0
        ASSERT_EQ(data, 0x00U);
    }

    PASS();
}

TEST dzTestChipReset(void) {
    ASSERT_NEQ(chip, NULL);

    {
        ASSERT_EQ(dzChipGetRB(chip), 1U);

        dzChipSetCLE(chip, 1U);
        dzChipToggleWE(chip);

        dzChipWrite(chip, DZ_CHIP_CMD_RESET, 100U);

        ASSERT_EQ(dzChipGetRB(chip), 0U);
    }

    PASS();
}

TEST dzTestChipReadID(void) {
    ASSERT_NEQ(chip, NULL);

    {
        ASSERT_EQ(dzChipGetRB(chip), 1U);

        dzChipSetCLE(chip, 1U);
        dzChipToggleWE(chip);

        dzChipWrite(chip, DZ_CHIP_CMD_READ_ID, 100U);

        dzChipSetALE(chip, 1U);
        dzChipToggleWE(chip);

        // NOTE: Undefined behavior
        dzChipWrite(chip, 0xFFU, 150U);
    }

    {
        dzChipSetALE(chip, 1U);
        dzChipToggleWE(chip);

        dzChipWrite(chip, 0x20U, 200U);

        dzChipSetALE(chip, 0U);
        dzChipSetCLE(chip, 0U);

        dzByte data = 0xFFU;

        const char onfiSignature[] = { 'O', 'N', 'F', 'I' };

        for (dzU32 i = 0U, j = sizeof onfiSignature / sizeof *onfiSignature;
             i < j;
             i++) {
            dzChipToggleRE(chip);

            dzChipRead(chip, &data, 300U + (i * 100U));

            ASSERT_EQ(data, onfiSignature[i]);
        }

        dzChipToggleRE(chip);

        dzChipRead(chip, &data, 700U);

        // NOTE: Undefined behavior
        ASSERT_EQ(data, 0xFFU);
    }

    {
        dzChipSetALE(chip, 1U);
        dzChipToggleWE(chip);

        dzChipWrite(chip, 0x00U, 1000U);

        dzByte data = 0xFFU;

        dzChipToggleRE(chip);

        dzChipRead(chip, &data, 1100U);

        ASSERT_EQ(data, 0x00U);
    }

    PASS();
}