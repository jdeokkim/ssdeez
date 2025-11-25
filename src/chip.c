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

/* Typedefs ===============================================================> */

// clang-format off

/* An enumeration that represents the current state of a NAND flash die. */
typedef enum dzChipState_ {
    DZ_CHIP_STATE_GF_RETRIEVE_PARAMS,
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

/* A structure that represents the metadata of a NAND flash chip. */
typedef struct dzChipMetadata_ {
    dzTimestamp currentTime;
    dzTimestamp remainingTime;
} dzChipMetadata;

/* A structure that represents a NAND flash chip. */
struct dzChip_ {
    dzChipConfig config;
    dzChipCommand command;
    dzChipMetadata metadata;
    dzByte address[7];
    dzByte addressCycleCount;
    dzDie **dies;
    dzChipCtrlLines lines;
    dzByte isReady;                          // R/B# (`0U` = BUSY)
    dzUSize offset;                          // R/W Position Indicator
    dzChipState state;
    dzByte timingMode;
    dzByte dieIndex;
};

// clang-format on

/* Constants ==============================================================> */

/* 
    Maximum "busy time for 'Get Features' and 'Set Features' time", 
    in microseconds. 
*/
static const dzU16 tFEAT = 1U;

/* ------------------------------------------------------------------------> */

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

/* Performs a "Get Features" operation on `chip`. */
static void dzChipGetFeatures(dzChip *chip, dzByte *result);

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
        if (config.isVerbose)
            DZ_API_INFO("initializing chip #%lu\n", config.chipId);

        newChip->config = config;

        newChip->command = DZ_CHIP_CMD_UNKNOWN;

        newChip->metadata.currentTime = 1U;
        newChip->metadata.remainingTime = 0U;

        dzUSize byteCount = sizeof newChip->address
                            / sizeof *(newChip->address);

        for (dzUSize i = 0U; i < byteCount; i++)
            newChip->address[i] = 0U;

        newChip->addressCycleCount = 0U;

        newChip->dies = malloc(config.dieCount * sizeof *(newChip->dies));

        for (dzU32 i = 0U; i < config.dieCount; i++) {
            config.dieConfig.dieId = i;

            if (config.isVerbose) DZ_API_INFO("initializing die #%u\n", i);

            dzResult dieInitResult = dzDieInit(&(newChip->dies[i]),
                                               config.dieConfig);

            if (dieInitResult != DZ_RESULT_OK) {
                free(newChip->dies), free(newChip);

                return dieInitResult;
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

        newChip->dieIndex = 0U;
        newChip->timingMode = 0U;

        dzChipPowerOnReset(newChip);
    }

    if (config.isVerbose) DZ_API_INFO("chip #%lu is ready\n", config.chipId);

    *chip = newChip;

    return DZ_RESULT_OK;
}

/* Releases the memory allocated for `chip`. */
void dzChipDeinit(dzChip *chip) {
    if (chip == NULL) return;

    if (chip->config.isVerbose)
        DZ_API_INFO("deinitializing chip #%lu\n", chip->config.chipId);

    for (dzU32 i = 0; i < chip->config.dieCount; i++)
        dzDieDeinit(chip->dies[i]);

    free(chip->dies), free(chip);
}

/* Reads `data` from `chip`'s I/O bus. */
void dzChipRead(dzChip *chip, dzByte *data, dzTimestamp ts) {
    if (chip == NULL || chip->lines.chipEnable
        || chip->lines.readEnable != 0xFFU || data == NULL)
        return;

    if (chip->config.isVerbose)
        DZ_API_INFO("reading incoming data from chip #%lu\n",
                    chip->config.chipId);

    switch (chip->command) {
        case DZ_CHIP_CMD_GET_FEATURES:
            dzChipGetFeatures(chip, data);

            break;

        case DZ_CHIP_CMD_READ_ID:
            dzChipReadID(chip, data);

            break;

        default:
            DZ_API_UNIMPLEMENTED();
    }

    chip->metadata.currentTime = ts;
    chip->lines.readEnable = 1U;
}

/* Writes `data` to `chip`'s I/O bus. */
void dzChipWrite(dzChip *chip, dzByte data, dzTimestamp ts) {
    if (chip == NULL || chip->lines.chipEnable
        || chip->lines.writeEnable != 0xFFU
        || (chip->lines.addrLatchEnable && chip->lines.cmdLatchEnable))
        return;

    if (chip->config.isVerbose)
        DZ_API_INFO("writing 0x%02X to chip #%lu\n",
                    data,
                    chip->config.chipId);

    if (chip->lines.addrLatchEnable) {
        dzChipWriteAddress(chip, data, ts);
    } else if (chip->lines.cmdLatchEnable) {
        dzChipDecodeCommand(chip, data, ts);
    } else {
        DZ_API_UNIMPLEMENTED();
    }

    chip->metadata.currentTime = ts;
    chip->lines.writeEnable = 1U;
}

/* Waits until `chip` is ready. */
dzTimestamp dzChipWaitUntilReady(dzChip *chip) {
    if (chip == NULL || dzChipIsReady(chip)) return 0U;

    chip->metadata.currentTime += chip->metadata.remainingTime;

    chip->metadata.remainingTime = 0U;

    return chip->metadata.currentTime;
}

/* ------------------------------------------------------------------------> */

/* Returns the current timestamp of `chip`, in microseconds. */
dzTimestamp dzChipGetCurrentTime(const dzChip *chip) {
    return (chip != NULL) ? chip->metadata.currentTime : 0U;
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
    if (state && chip->lines.cmdLatchEnable) {
        if (chip->config.isVerbose) {
            DZ_API_WARNING("ALE and CLE signals are mutually exclusive!\n");
            DZ_API_WARNING("set CLE%lu to HIGH (ACTIVE) -> (INACTIVE)\n",
                           chip->config.chipId);
        }

        chip->lines.cmdLatchEnable = 0U;
    }

    if (chip->config.isVerbose)
        DZ_API_INFO("set ALE%lu to %s\n",
                    chip->config.chipId,
                    state ? "HIGH (ACTIVE)" : "LOW (INACTIVE)");

    chip->lines.addrLatchEnable = state;
}

/* Sets the state of the "Command Latch Enable" control line in `chip`. */
void dzChipSetCLE(dzChip *chip, dzByte state) {
    if (chip == NULL) return;

    if (state && chip->lines.addrLatchEnable) {
        if (chip->config.isVerbose) {
            DZ_API_WARNING("ALE and CLE signals are mutually exclusive!\n");
            DZ_API_WARNING("set ALE%lu to HIGH (ACTIVE) -> (INACTIVE)\n",
                           chip->config.chipId);
        }

        chip->lines.addrLatchEnable = 0U;
    }

    if (chip->config.isVerbose)
        DZ_API_INFO("set CLE%lu to %s\n",
                    chip->config.chipId,
                    state ? "HIGH (ACTIVE)" : "LOW (INACTIVE)");

    chip->lines.cmdLatchEnable = state;
}

/* Sets the state of the "Chip Enable" control line in `chip`. */
void dzChipSetCE(dzChip *chip, dzByte state) {
    if (chip == NULL) return;

    if (chip->config.isVerbose)
        DZ_API_INFO("set CE%lu# to %s\n",
                    chip->config.chipId,
                    state ? "HIGH (INACTIVE)" : "LOW (ACTIVE)");

    chip->lines.chipEnable = state;
}

/* ------------------------------------------------------------------------> */

/* Toggles the state of the "Read Enable" control line in `chip`. */
void dzChipToggleRE(dzChip *chip) {
    if (chip == NULL) return;

    if (chip->config.isVerbose)
        DZ_API_INFO("set RE%lu# to LOW (ACTIVE) -> HIGH (INACTIVE)\n",
                    chip->config.chipId);

    chip->lines.readEnable = 0xFFU;
}

/* Toggles the state of the "Write Enable" control line in `chip`. */
void dzChipToggleWE(dzChip *chip) {
    if (chip == NULL) return;

    if (chip->config.isVerbose)
        DZ_API_INFO("set WE%lu# to LOW (ACTIVE) -> HIGH (INACTIVE)\n",
                    chip->config.chipId);

    chip->lines.writeEnable = 0xFFU;
}

/* Sets the state of the "Write Protect" control line in `chip`. */
void dzChipSetWP(dzChip *chip, dzByte state) {
    if (chip == NULL) return;

    chip->lines.writeProtect = state;

    // TODO: ?
    DZ_API_UNIMPLEMENTED();
}

/* Private Functions ======================================================> */

/* Performs `command` on `chip`. */
static void dzChipDecodeCommand(dzChip *chip, dzByte command, dzTimestamp ts) {
    chip->command = command;

    if (chip->config.isVerbose)
        DZ_API_INFO("decoded ONFI command 0x%02X\n", command);

    switch (command) {
        case DZ_CHIP_CMD_GET_FEATURES:
            if (chip->config.isVerbose)
                DZ_API_INFO("waiting for 1 address cycle\n");

            break;

        case DZ_CHIP_CMD_RESET:
            dzChipReset(chip, ts);

            break;

        case DZ_CHIP_CMD_READ_0:
            if (chip->config.isVerbose)
                DZ_API_INFO("waiting for 5 address cycles\n");

            break;

        case DZ_CHIP_CMD_READ_ID:
            if (chip->config.isVerbose)
                DZ_API_INFO("waiting for 1 address cycle\n");

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

    if (chip->config.isVerbose)
        DZ_API_INFO("received address 0x%02X\n", address);

    chip->address[chip->addressCycleCount++] = address;

    switch (chip->command) {
        case DZ_CHIP_CMD_GET_FEATURES:
            chip->addressCycleCount = 0U;

            chip->state = DZ_CHIP_STATE_GF_RETRIEVE_PARAMS;

            break;

        case DZ_CHIP_CMD_READ_0:
            // TODO: ...

            break;

        case DZ_CHIP_CMD_READ_ID:
            chip->addressCycleCount = 0U;

            break;

        default:
            DZ_API_UNIMPLEMENTED();
    }

    chip->metadata.currentTime = ts;
}

/* ------------------------------------------------------------------------> */

/* Performs a "Get Features" operation on `chip`. */
static void dzChipGetFeatures(dzChip *chip, dzByte *result) {
    // NOTE: Target-level Command
    if (!dzChipIsReady(chip)) {
        if (chip->config.isVerbose)
            DZ_API_WARNING("chip #%lu is busy\n", chip->config.chipId);

        return;
    }

    if (chip->address[0] == 0x01U) {
        if (chip->offset >= 4U) {
            *result = 0xFFU;

            chip->state = DZ_CHIP_STATE_IDLE;

            return;
        }

        *result = (chip->offset == 0U) ? chip->timingMode : 0x00U;

        chip->offset++;
    }

    if (chip->config.isVerbose)
        DZ_API_INFO("returned next byte 0x%02X\n", *result);

    chip->metadata.remainingTime = tFEAT;
}

/* Performs a "Power-On Reset" operation on `chip`. */
static void dzChipPowerOnReset(dzChip *chip) {
    chip->command = DZ_CHIP_CMD_RESET;

    for (dzByte i = 0U; i < chip->config.dieCount; i++) {
        if (chip->config.isVerbose)
            DZ_API_INFO("requesting a power-on reset to die #%u\n", i);

        dzDieDecodeCommand(chip->dies[i],
                           DZ_CHIP_CMD_RESET,
                           chip->metadata.currentTime);
    }

    dzTimestamp remainingTime = 0U;

    for (dzByte i = 0U; i < chip->config.dieCount; i++) {
        dzTimestamp tRST = dzDieWaitUntilReady(chip->dies[i]);

        if (remainingTime < tRST) remainingTime = tRST;
    }

    (void) dzChipWaitUntilReady(chip);

    chip->state = DZ_CHIP_STATE_IDLE;
}

/* Performs a "Read ID" operation on `chip`. */
static void dzChipReadID(dzChip *chip, dzByte *result) {
    // NOTE: Target-level Command
    if (!dzChipIsReady(chip)) {
        if (chip->config.isVerbose)
            DZ_API_WARNING("chip #%lu is busy\n", chip->config.chipId);

        return;
    }

    if (chip->address[0] == 0x00U) {
        // NOTE: JEDEC Manufacturer ID and Device ID
        *result = 0x00U;

        if (chip->config.isVerbose)
            DZ_API_INFO("returned next byte 0x%02X\n", *result);
    } else if (chip->address[0] == 0x20U) {
        if (chip->offset >= sizeof onfiSignature / sizeof *onfiSignature) {
            *result = 0xFFU;

            chip->state = DZ_CHIP_STATE_IDLE_RD;

            return;
        }

        *result = onfiSignature[chip->offset++];

        if (chip->config.isVerbose)
            DZ_API_INFO("returned next ONFI signature byte 0x%02X ('%c')\n",
                        *result,
                        *result);
    } else {
        if (chip->config.isVerbose)
            DZ_API_WARNING("ignored invalid request with address 0x%02X\n",
                           chip->address[0]);
    }
}

/* Performs a "Reset" operation on `chip`. */
static void dzChipReset(dzChip *chip, dzTimestamp ts) {
    // NOTE: Target-level Command
    if (!dzChipIsReady(chip)) {
        if (chip->config.isVerbose)
            DZ_API_WARNING("chip #%lu is busy\n", chip->config.chipId);

        return;
    }

    for (dzByte i = 0U; i < chip->config.dieCount; i++) {
        if (chip->config.isVerbose)
            DZ_API_INFO("requesting a reset to die #%u\n", i);

        dzDieDecodeCommand(chip->dies[i], DZ_CHIP_CMD_RESET, ts);
    }
}

/* ------------------------------------------------------------------------> */

/* Returns `1U` if `chip` is ready. */
DZ_API_STATIC_INLINE dzByte dzChipIsReady(dzChip *chip) {
    dzByte result = 1U;

    for (dzByte i = 0U; i < chip->config.dieCount; i++)
        result &= dzDieGetRDY(chip->dies[i]);

    return (chip->isReady = result);
}
