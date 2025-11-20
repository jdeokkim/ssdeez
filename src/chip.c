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
    dzByte chipEnable;       // CE# (`0U` = ACTIVE)
    dzByte addrLatchEnable;  // ALE (`1U` = ACTIVE)
    dzByte cmdLatchEnable;   // CLE (`1U` = ACTIVE)
    dzByte readEnable;       // RE# (`0U` = ACTIVE)
    dzByte writeEnable;      // WE# (`0U` = ACTIVE)
    dzByte writeProtect;     // WP# (`0U` = ACTIVE)
    dzByte lockEnable;       // LOCK (`1U` = ACTIVE)
} dzChipCtrlLines;

/* A structure that represents a NAND flash chip. */
struct dzChip_ {
    dzChipConfig config;
    dzDie **dies;
    dzChipCtrlLines lines;
    dzByte readyBusy;  // R/B# (`0U` = ACTIVE)
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

/* Performs a "Reset" operation on `chip`. */
static void dzChipReset(dzChip *chip);

// TODO: dzChipSetFeatures()

/* Performs `command` on `chip`. */
static void dzChipDecodeCommand(dzChip *chip, dzByte command);

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

        newChip->lines = (dzChipCtrlLines) { .addrLatchEnable = 0U,
                                             .cmdLatchEnable = 0U,
                                             .chipEnable = 1U,
                                             .readEnable = 1U,
                                             .writeEnable = 1U,
                                             .writeProtect = 1U };

        newChip->readyBusy = 0U;
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

/* Writes `data` to `chip`'s I/O bus. */
void dzChipWrite(dzChip *chip, dzByte data) {
    if (chip == NULL || chip->readyBusy
        || (chip->lines.addrLatchEnable && chip->lines.cmdLatchEnable)
        || chip->lines.chipEnable || chip->lines.writeEnable)
        return;

    // dzChipSetWE(chip, 0U), dzChipSetWE(chip, 1U);

    if (chip->lines.addrLatchEnable) {
        DZ_API_UNIMPLEMENTED();
    } else if (chip->lines.cmdLatchEnable) {
        dzChipDecodeCommand(chip, data);
    } else {
        DZ_API_UNIMPLEMENTED();
    }
}

/* ------------------------------------------------------------------------> */

/* Returns the state of the "Address Latch Enable" control line in `chip`. */
dzByte dzChipGetALE(const dzChip *chip) {
    return (chip != NULL) ? chip->lines.addrLatchEnable : 0U;
}

/* Returns the state of the "Command Latch Enable" control line in `chip`. */
dzByte dzChipGetCLE(const dzChip *chip) {
    return (chip != NULL) ? chip->lines.cmdLatchEnable : 0U;
}

/* Returns the state of the "Chip Enable" control line in `chip`. */
dzByte dzChipGetCE(const dzChip *chip) {
    return (chip != NULL) ? chip->lines.chipEnable : 1U;
}

/* Returns the state of the "Read Enable" control line in `chip`. */
dzByte dzChipGetRE(const dzChip *chip) {
    return (chip != NULL) ? chip->lines.readEnable : 1U;
}

/* Returns the state of the "Write Enable" control line in `chip`. */
dzByte dzChipGetWE(const dzChip *chip) {
    return (chip != NULL) ? chip->lines.writeEnable : 1U;
}

/* Returns the state of the "Ready/Busy" control line in `chip`. */
dzByte dzChipGetRB(const dzChip *chip) {
    return (chip != NULL) ? chip->readyBusy : 1U;
}

/* ------------------------------------------------------------------------> */

/* Sets the state of the "Address Latch Enable" control line in `chip`. */
void dzChipSetALE(dzChip *chip, dzByte state) {
    if (chip == NULL) return;

    // NOTE: The ALE and CLE signals are mutually exclusive
    if (state && chip->lines.cmdLatchEnable) chip->lines.cmdLatchEnable = 0U;

    chip->lines.addrLatchEnable = state;
}

/* Sets the state of the "Command Latch Enable" control line in `chip`. */
void dzChipSetCLE(dzChip *chip, dzByte state) {
    if (chip == NULL) return;

    // NOTE: The ALE and CLE signals are mutually exclusive
    if (state && chip->lines.addrLatchEnable) chip->lines.addrLatchEnable = 0U;

    chip->lines.cmdLatchEnable = state;
}

/* Sets the state of the "Chip Enable" control line in `chip`. */
void dzChipSetCE(dzChip *chip, dzByte state) {
    if (chip == NULL) return;

    chip->lines.chipEnable = state;
}

/* Sets the state of the "Read Enable" control line in `chip`. */
void dzChipSetRE(dzChip *chip, dzByte state) {
    if (chip == NULL) return;

    chip->lines.readEnable = state;
}

/* Sets the state of the "Write Enable" control line in `chip`. */
void dzChipSetWE(dzChip *chip, dzByte state) {
    if (chip == NULL) return;

    chip->lines.writeEnable = state;
}

/* Sets the state of the "Write Protect" control line in `chip`. */
void dzChipSetWP(dzChip *chip, dzByte state) {
    if (chip == NULL) return;

    chip->lines.writeProtect = state;
}

/* Private Functions ======================================================> */

/* Performs `command` on `chip`. */
static void dzChipDecodeCommand(dzChip *chip, dzByte command) {
    switch (command) {
        case DZ_CHIP_CMD_RESET:
            dzChipReset(chip);

            break;

        default:
            DZ_API_UNIMPLEMENTED();
    }
}

/* ------------------------------------------------------------------------> */

/* Performs a "Reset" operation on `chip`. */
static void dzChipReset(dzChip *chip) {
    for (dzByte i = 0U; i < chip->config.dieCount; i++)
        dzDieReset(chip->dies[i]);
}