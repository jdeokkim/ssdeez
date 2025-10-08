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

/* Private Function Prototypes ============================================> */

TEST dzTestPageOps(void);

/* Public Functions =======================================================> */

SUITE(dzTestDieOps) {
    RUN_TEST(dzTestPageOps);
}

/* Private Functions ======================================================> */

TEST dzTestPageOps(void) {
    dzDieConfig dieConfig = { .cellType = DZ_CELL_TYPE_MLC,
                              .planeCountPerDie = 2U,
                              .blockCountPerPlane = 4U,
                              .layerCountPerBlock = 5U,
                              .pageSizeInBytes = 16U };

    dzDie *die = dzDieCreate(dieConfig);

    {
        dzU64 pageCount = dzDieGetPageCount(die);

        // clang-format off

        ASSERT_EQ(
            (dieConfig.planeCountPerDie 
                * dieConfig.blockCountPerPlane
                * dieConfig.layerCountPerBlock),
            pageCount
        );

        // clang-format on
    }

    {
        // clang-format off

        const dzByte srcBuffer[] = { 
            0x12, 0x34, 0x56, 0x78, 0x91, 0x23, 0x45, 0x67,
            0x89, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23
        };

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
    }

    dzDieRelease(die);

    PASS();
}
