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

// NOTE: K9F2G08U0M
static const dzDieConfig dieConfig = { .dieId = 0U,
                                       .cellType = DZ_CELL_TYPE_SLC,
                                       .badBlockRatio = 0.01f,
                                       .planeCountPerDie = 1U,
                                       .blockCountPerPlane = 2048U,
                                       .pageCountPerBlock = 64U,
                                       .pageSizeInBytes = 2048U };

/* Private Variables ======================================================> */

static dzDie *die = NULL;

/* Private Function Prototypes ============================================> */

static void dzTestSetupCb(void *ctx);
static void dzTestTeardownCb(void *ctx);

TEST dzTestDieCommands(void);
TEST dzTestDieStats(void);

/* Public Functions =======================================================> */

SUITE(dzTestDie) {
    SET_SETUP(dzTestSetupCb, NULL);
    SET_TEARDOWN(dzTestTeardownCb, NULL);

    RUN_TEST(dzTestDieCommands);
    RUN_TEST(dzTestDieStats);
}

/* Private Functions ======================================================> */

static void dzTestSetupCb(void *ctx) {
    DZ_API_UNUSED_VARIABLE(ctx);

    (void) dzDieInit(&die, dieConfig);
}

static void dzTestTeardownCb(void *ctx) {
    DZ_API_UNUSED_VARIABLE(ctx);

    dzDieDeinit(die), die = NULL;
}

/* ========================================================================> */

TEST dzTestDieCommands(void) {
    ASSERT_NEQ(die, NULL);

    // TODO: ...

    PASS();
}

TEST dzTestDieStats(void) {
    ASSERT_NEQ(die, NULL);

    // TODO: ...

    PASS();
}