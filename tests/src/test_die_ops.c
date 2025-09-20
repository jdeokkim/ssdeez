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

TEST dzTestCellRw(void);

/* Public Functions =======================================================> */

SUITE(dzTestDieOps) {
    RUN_TEST(dzTestCellRw);
}

/* Private Functions ======================================================> */

TEST dzTestCellRw(void) {
    dzDie *die = dzDieCreate((dzDieConfig) { .cellType = DZ_CELL_TLC,
                                             .planeCountPerDie = 1,
                                             .blockCountPerPlane = 8,
                                             .layerCountPerBlock = 1,
                                             .pageCountPerLayer = 4,
                                             .cellCountPerPage = 32 });

    dzDieRelease(die);

    PASS();
}
