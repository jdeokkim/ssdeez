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

/* A structure that represents a NAND flash chip. */
struct dzChip_ {
    dzChipConfig config;
    dzDie **dies;
    bool isReady;  // R/B#
    // TODO: ...
};

/* Constants ==============================================================> */

/* A constant that represents an invalid chip identifier. */
const dzU64 DZ_CHIP_INVALID_ID = UINT64_MAX;

/* Private Variables ======================================================> */

// TODO: ...

/* Private Function Prototypes ============================================> */

// TODO: ...

/* Public Functions =======================================================> */

/* Initializes `*chip` with the given `config`. */
dzResult dzChipInit(dzChip **chip, dzChipConfig config) {
    // clang-format off

    if (chip == NULL
        || config.dieConfig == NULL
        || config.chipId == DZ_CHIP_INVALID_ID
        || config.dieCount == 0U)
        return DZ_RESULT_INVALID_ARGUMENT;

    // clang-format on

    dzChip *newChip = malloc(sizeof *newChip);

    if (newChip == NULL) return DZ_RESULT_NO_MEMORY;

    {
        newChip->config = config;

        newChip->dies = malloc(config.dieCount * sizeof *(newChip->dies));

        for (dzU32 i = 0; i < config.dieCount; i++) {
            config.dieConfig->dieId = i;

            if (dzDieInit(&(newChip->dies[i]), *(config.dieConfig))
                != DZ_RESULT_OK) {
                free(newChip->dies), free(newChip);

                return DZ_RESULT_INTERNAL_ERROR;
            }
        }

        newChip->isReady = true;
    }

    *chip = newChip;

    return DZ_RESULT_OK;
}

/* Releases the memory allocated for `chip`. */
void dzChipDeinit(dzChip *chip) {
    if (chip == NULL) return;

    for (dzU32 i = 0; i < chip->config.dieCount; i++)
        dzDieDeinit(chip->dies[i]);

    free(chip->dies), free(chip);
}
