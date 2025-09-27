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

TEST dzTestDieRw(void);

/* Public Functions =======================================================> */

SUITE(dzTestDieOps) {
    RUN_TEST(dzTestDieRw);
}

/* Private Functions ======================================================> */

TEST dzTestDieRw(void) {
    dzDie *die = dzDieCreate(
        (dzDieConfig) {
            .cellType = DZ_CELL_TYPE_MLC,
            .planeCountPerDie = 2U,
            .blockCountPerPlane = 4U,
            .layerCountPerBlock = 5U,
            .pageSizeInBytes = 16U
        }
    );

    // TODO: ...

    dzDieRelease(die);

    PASS();
}
