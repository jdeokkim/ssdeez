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

/* A structure that represents the control lines of a NAND flash chip. */
typedef struct dzChipCtrlLines_ {
    dzBool chipEnable;       // CE# (`false` = ACTIVE)
    dzBool addrLatchEnable;  // ALE (`true` = ACTIVE)
    dzBool cmdLatchEnable;   // CLE (`true` = ACTIVE)
    dzBool readEnable;       // RE# (`false` = ACTIVE)
    dzBool writeEnable;      // WE# (`false` = ACTIVE)
    dzBool writeProtect;     // WP# (`false` = ACTIVE)
} dzChipCtrlLines;

/* A structure that represents a NAND flash chip. */
struct dzChip_ {
    dzChipConfig config;
    dzDie **dies;
    dzChipCtrlLines lines;
    dzBool readyBusy;  // R/B# (`false` = ACTIVE)
    // TODO: ...
};

/* Constants ==============================================================> */

// TODO: ...

/* Private Variables ======================================================> */

// TODO: ...

/* Private Function Prototypes ============================================> */

// TODO: dzChipBlockErase()

// TODO: dzChipChangeReadColumn()

// TODO: dzChipChangeWriteColumn()

// TODO: dzChipCopyback()

// TODO: dzChipGetFeatures()

// TODO: dzChipPageProgram()

// TODO: dzChipRead()

// TODO: dzChipReadCache()

// TODO: dzChipReadID()

// TODO: dzChipReadParameterPage()

// TODO: dzChipReadStatus()

// TODO: dzChipReadStatusEnhanced()

// TODO: dzChipReadUniqueID()

// TODO: dzChipReset()

// TODO: dzChipSetFeatures()

/* Public Functions =======================================================> */

/* Initializes `*chip` with the given `config`. */
dzResult dzChipInit(dzChip **chip, dzChipConfig config) {
    // clang-format off

    if (chip == NULL
        || config.chipId == DZ_API_INVALID_ID
        || config.dieCount == 0U)
        return DZ_RESULT_INVALID_ARGUMENT;

    // clang-format on

    dzChip *newChip = malloc(sizeof *newChip);

    if (newChip == NULL) return DZ_RESULT_OUT_OF_MEMORY;

    {
        newChip->config = config;

        newChip->dies = malloc(config.dieCount * sizeof *(newChip->dies));

        for (dzU32 i = 0; i < config.dieCount; i++) {
            config.dieConfig.dieId = i;

            if (dzDieInit(&(newChip->dies[i]), config.dieConfig)
                != DZ_RESULT_OK) {
                free(newChip->dies), free(newChip);

                return DZ_RESULT_UNKNOWN;
            }
        }

        newChip->lines = (dzChipCtrlLines) { .addrLatchEnable = false,
                                             .cmdLatchEnable = false,
                                             .chipEnable = true,
                                             .readEnable = true,
                                             .writeEnable = true,
                                             .writeProtect = true };

        newChip->readyBusy = false;
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

/* ------------------------------------------------------------------------> */

/* Returns the state of the "Address Latch Enable" control line in `chip`. */
dzBool dzChipGetALE(const dzChip *chip) {
    return (chip != NULL) ? chip->lines.addrLatchEnable : false;
}

/* Returns the state of the "Command Latch Enable" control line in `chip`. */
dzBool dzChipGetCLE(const dzChip *chip) {
    return (chip != NULL) ? chip->lines.cmdLatchEnable : false;
}

/* Returns the state of the "Chip Enable" control line in `chip`. */
dzBool dzChipGetCE(const dzChip *chip) {
    return (chip != NULL) ? chip->lines.chipEnable : true;
}

/* Returns the state of the "Read Enable" control line in `chip`. */
dzBool dzChipGetRE(const dzChip *chip) {
    return (chip != NULL) ? chip->lines.readEnable : true;
}

/* Returns the state of the "Write Enable" control line in `chip`. */
dzBool dzChipGetWE(const dzChip *chip) {
    return (chip != NULL) ? chip->lines.writeEnable : true;
}

/* Returns the state of the "Ready/Busy" control line in `chip`. */
dzBool dzChipGetRB(const dzChip *chip) {
    return (chip != NULL) ? chip->readyBusy : true;
}

/* ------------------------------------------------------------------------> */

/* Sets the state of the "Address Latch Enable" control line in `chip`. */
void dzChipSetALE(dzChip *chip, dzBool state) {
    if (chip == NULL) return;

    if (state && chip->lines.cmdLatchEnable)
        chip->lines.cmdLatchEnable = false;

    chip->lines.addrLatchEnable = state;
}

/* Sets the state of the "Command Latch Enable" control line in `chip`. */
void dzChipSetCLE(dzChip *chip, dzBool state) {
    if (chip == NULL) return;

    if (state && chip->lines.addrLatchEnable)
        chip->lines.addrLatchEnable = false;

    chip->lines.cmdLatchEnable = state;
}

/* Sets the state of the "Chip Enable" control line in `chip`. */
void dzChipSetCE(dzChip *chip, dzBool state) {
    if (chip == NULL) return;

    chip->lines.chipEnable = state;
}

/* Sets the state of the "Read Enable" control line in `chip`. */
void dzChipSetRE(dzChip *chip, dzBool state) {
    if (chip == NULL) return;

    chip->lines.readEnable = state;
}

/* Sets the state of the "Write Enable" control line in `chip`. */
void dzChipSetWE(dzChip *chip, dzBool state) {
    if (chip == NULL) return;

    chip->lines.writeEnable = state;
}

/* Sets the state of the "Write Protect" control line in `chip`. */
void dzChipSetWP(dzChip *chip, dzBool state) {
    if (chip == NULL) return;

    chip->lines.writeProtect = state;
}