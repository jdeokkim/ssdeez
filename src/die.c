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

#include "ssdeez.h"

/* Macros =================================================================> */

// TODO: ...

/* Typedefs ===============================================================> */

/* A structure that represents a group of NAND flash planes. */
struct dzDie_ {
    dzDieConfig config;     // The configuration of this die
    unsigned char *buffer;  // The buffer used to access plane, block, etc.
    // TODO: ...
};

/* Private Function Prototypes ============================================> */

// TODO: ...

/* Constants ==============================================================> */

// TODO: ...

/* Private Variables ======================================================> */

// TODO: ...

/* Public Functions =======================================================> */

/* Creates a die with the given configuration. */
dzDie *dzDieCreate(dzDieConfig config) {
    dzDie *die = malloc(sizeof *die);

    die->config = config;

    // clang-format off

    size_t bufferSize = (
        config.planeCountPerDie 
            * config.blockCountPerPlane
            * config.pageCountPerBlock 
            * config.pageSizeInBytes
    );

    // clang-format on

    die->buffer = malloc(bufferSize * sizeof *(die->buffer));

    return die;
}

/* Releases the memory allocated for `die`. */
void dzDieRelease(dzDie *die) {
    if (die == NULL) return;

    free(die->buffer), free(die);
}
