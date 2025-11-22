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

// clang-format off

/* An enumeration that represents the current state of a NAND flash die. */
typedef enum dzChipState_ {
    DZ_CHIP_STATE_IDLE,
    DZ_CHIP_STATE_IDLE_RD
} dzChipState;

/* ------------------------------------------------------------------------> */

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
    dzChipCommand command;
    dzTimestamp currentTime;
    dzByte address[7], addressCycleCount;  
    dzDie **dies;
    dzChipCtrlLines lines;
    dzByte isReady;                        // R/B# (`0U` = BUSY)
    dzUSize offset;                        // R/W Position Indicator
    dzChipState state;
};

// clang-format on

/* Constants ==============================================================> */

/* ONFI signature bytes. */
static const dzByte onfiSignature[] = { 'O', 'N', 'F', 'I' };

/* Private Variables ======================================================> */

// TODO: ...

/* Private Function Prototypes ============================================> */

/* Performs `command` on `chip`. */
static void dzChipDecodeCommand(dzChip *chip, dzByte command, dzTimestamp ts);

/* Writes `address` to `chip`'s address register. */
static void dzChipWriteAddress(dzChip *chip, dzByte address, dzTimestamp ts);

/* ------------------------------------------------------------------------> */

// TODO: dzChipBlockErase()

// TODO: dzChipChangeReadColumn()

// TODO: dzChipChangeWriteColumn()

// TODO: dzChipCopyback()

// TODO: dzChipGetFeatures()

// TODO: dzChipPageProgram()

/* Performs a "Power-On Reset" operation on `chip`. */
static void dzChipPowerOnReset(dzChip *chip);

// TODO: dzChipRead()

// TODO: dzChipReadCache()

/* Performs a "Read ID" operation on `chip`. */
static void dzChipReadID(dzChip *chip, dzByte *result);

// TODO: dzChipReadParameterPage()

// TODO: dzChipReadStatus()

// TODO: dzChipReadStatusEnhanced()

// TODO: dzChipReadUniqueID()

/* Performs a "Reset" operation on `chip`. */
static void dzChipReset(dzChip *chip, dzTimestamp ts);

// TODO: dzChipSetFeatures()

/* ------------------------------------------------------------------------> */

/* Returns `1U` if `chip` is ready. */
DZ_API_STATIC_INLINE dzByte dzChipIsReady(dzChip *chip);

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

        newChip->command = DZ_CHIP_CMD_UNKNOWN;
        newChip->currentTime = 1U;

        dzUSize byteCount = sizeof newChip->address
                            / sizeof *(newChip->address);

        for (dzUSize i = 0U; i < byteCount; i++)
            newChip->address[i] = 0U;

        newChip->addressCycleCount = 0U;

        newChip->dies = malloc(config.dieCount * sizeof *(newChip->dies));

        for (dzU32 i = 0U; i < config.dieCount; i++) {
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

        newChip->isReady = 1U;
        newChip->offset = 0U;

        newChip->state = DZ_CHIP_STATE_IDLE;

        dzChipPowerOnReset(newChip);
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

/* Reads `data` from `chip`'s I/O bus. */
void dzChipRead(dzChip *chip, dzByte *data, dzTimestamp ts) {
    if (chip == NULL || chip->lines.chipEnable
        || chip->lines.readEnable != 0xFFU || data == NULL)
        return;

    switch (chip->command) {
        case DZ_CHIP_CMD_READ_ID:
            dzChipReadID(chip, data);

            break;

        default:
            DZ_API_UNIMPLEMENTED();
    }

    chip->currentTime = ts;
    chip->lines.readEnable = 1U;
}

/* Writes `data` to `chip`'s I/O bus. */
void dzChipWrite(dzChip *chip, dzByte data, dzTimestamp ts) {
    if (chip == NULL || chip->lines.chipEnable
        || chip->lines.writeEnable != 0xFFU
        || (chip->lines.addrLatchEnable && chip->lines.cmdLatchEnable))
        return;

    if (chip->lines.addrLatchEnable) {
        dzChipWriteAddress(chip, data, ts);
    } else if (chip->lines.cmdLatchEnable) {
        dzChipDecodeCommand(chip, data, ts);
    } else {
        DZ_API_UNIMPLEMENTED();
    }

    chip->currentTime = ts;
    chip->lines.writeEnable = 1U;
}

/* ------------------------------------------------------------------------> */

/* Returns the state of the "Address Latch Enable" control line in `chip`. */
dzByte dzChipGetALE(const dzChip *chip) {
    return (chip != NULL) ? !!chip->lines.addrLatchEnable : 0U;
}

/* Returns the state of the "Command Latch Enable" control line in `chip`. */
dzByte dzChipGetCLE(const dzChip *chip) {
    return (chip != NULL) ? !!chip->lines.cmdLatchEnable : 0U;
}

/* Returns the state of the "Chip Enable" control line in `chip`. */
dzByte dzChipGetCE(const dzChip *chip) {
    return (chip != NULL) ? !!chip->lines.chipEnable : 1U;
}

/* Returns the state of the "Ready/Busy" control line in `chip`. */
dzByte dzChipGetRB(const dzChip *chip) {
    return (chip != NULL) ? dzChipIsReady((dzChip *) chip) : 0U;
}

/* Returns the state of the "Write Protect" control line in `chip`. */
dzByte dzChipGetWP(const dzChip *chip) {
    return (chip != NULL) ? !!chip->lines.writeProtect : 1U;
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

/* ------------------------------------------------------------------------> */

/* Toggles the state of the "Read Enable" control line in `chip`. */
void dzChipToggleRE(dzChip *chip) {
    if (chip == NULL) return;

    chip->lines.readEnable = 0xFFU;
}

/* Toggles the state of the "Write Enable" control line in `chip`. */
void dzChipToggleWE(dzChip *chip) {
    if (chip == NULL) return;

    chip->lines.writeEnable = 0xFFU;
}

/* Sets the state of the "Write Protect" control line in `chip`. */
void dzChipSetWP(dzChip *chip, dzByte state) {
    if (chip == NULL) return;

    chip->lines.writeProtect = state;

    // TODO: ?
}

/* Private Functions ======================================================> */

/* Performs `command` on `chip`. */
static void dzChipDecodeCommand(dzChip *chip, dzByte command, dzTimestamp ts) {
    chip->command = command;

    switch (command) {
        case DZ_CHIP_CMD_RESET:
            dzChipReset(chip, ts);

            break;

        case DZ_CHIP_CMD_READ_ID:
            // NOTE: Wait for 1 address cycle

            break;

        default:
            DZ_API_UNIMPLEMENTED();
    }
}

/* Writes `address` to `chip`'s address register. */
static void dzChipWriteAddress(dzChip *chip, dzByte address, dzTimestamp ts) {
    if (chip->addressCycleCount
        >= (sizeof chip->address / sizeof *(chip->address)))
        return;

    DZ_API_UNUSED_VARIABLE(ts);

    chip->address[chip->addressCycleCount++] = address;

    switch (chip->command) {
        case DZ_CHIP_CMD_READ_ID:
            chip->addressCycleCount = 0U;

            break;

        default:
            DZ_API_UNIMPLEMENTED();
    }
}

/* ------------------------------------------------------------------------> */

/* Performs a "Power-On Reset" operation on `chip`. */
static void dzChipPowerOnReset(dzChip *chip) {
    chip->command = DZ_CHIP_CMD_RESET;

    for (dzByte i = 0U; i < chip->config.dieCount; i++)
        dzDieDecodeCommand(chip->dies[i],
                           DZ_CHIP_CMD_RESET,
                           chip->currentTime);

    dzTimestamp elapsedTime = 0U;

    for (dzByte i = 0U; i < chip->config.dieCount; i++) {
        dzTimestamp tRST = dzDieWaitUntilRDY(chip->dies[i]);

        if (elapsedTime < tRST) elapsedTime = tRST;
    }

    chip->currentTime += elapsedTime;

    chip->state = DZ_CHIP_STATE_IDLE;
}

/* Performs a "Read ID" operation on `chip`. */
static void dzChipReadID(dzChip *chip, dzByte *result) {
    if (chip->address[0] == 0x00U) {
        // NOTE: JEDEC Manufacturer ID and Device ID
        *result = 0x00U;
    } else if (chip->address[0] == 0x20U) {
        if (chip->offset >= sizeof onfiSignature / sizeof *onfiSignature) {
            *result = 0xFFU;

            chip->state = DZ_CHIP_STATE_IDLE_RD;

            return;
        }

        *result = onfiSignature[chip->offset++];
    }
}

/* Performs a "Reset" operation on `chip`. */
static void dzChipReset(dzChip *chip, dzTimestamp ts) {
    // NOTE: Target-level Command
    if (!chip->isReady) return;

    for (dzByte i = 0U; i < chip->config.dieCount; i++)
        dzDieDecodeCommand(chip->dies[i], DZ_CHIP_CMD_RESET, ts);
}

/* ------------------------------------------------------------------------> */

/* Returns `1U` if `chip` is ready. */
DZ_API_STATIC_INLINE dzByte dzChipIsReady(dzChip *chip) {
    dzByte result = 1U;

    for (dzByte i = 0U; i < chip->config.dieCount; i++)
        result &= dzDieGetRDY(chip->dies[i]);

    return (chip->isReady = result);
}
