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

#ifndef SSDEEZ_H
#define SSDEEZ_H

#ifdef __cplusplus
extern "C" {
#endif  // `__cplusplus`

/* Includes ==============================================================> */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* Macros =================================================================> */

/* Typedefs ===============================================================> */

/* Aliases for primitive integer types. */

typedef bool          dzBool;

typedef unsigned char dzByte;

typedef int32_t       dzI32;
typedef int64_t       dzI64;

typedef uint32_t      dzU32;
typedef uint64_t      dzU64;

/* ========================================================================> */

/* An enumeration that represents the type of a NAND flash cell. */
typedef enum dzCellType_ {
    DZ_CELL_UNKNOWN,
    DZ_CELL_SLC,  // 1 voltage state
    DZ_CELL_DLC,  // 2 voltage states
    DZ_CELL_TLC,  // 3 voltage states
    DZ_CELL_QLC   // 4 voltage states
} dzCellType;

/* ========================================================================> */

/* 
    A structure that represents an interface between the controller 
    and a group of dies. 
*/
typedef struct dzChannel_ dzChannel;

/* ========================================================================> */

/* A structure that represents a group of NAND flash planes. */
typedef struct dzDie_ dzDie;

/* A structure that represents the configuration of a `dzDie`. */
typedef struct dzDieConfig_ {
    dzCellType cellType;
    dzU32 planeCountPerDie;
    dzU32 blockCountPerPlane;
    dzU32 layerCountPerBlock;  // NOTE: V-NAND (3D NAND)
    dzU32 pageCountPerLayer;
    dzU32 cellCountPerPage;
} dzDieConfig;

/* Public Functions =======================================================> */

/* <-------------------------------------------------------- [src/channel.c] */

/* Creates a channel with the given `id`. */
dzChannel *dzChannelCreate(dzI32 id);

/* Releases the memory allocated for the `channel`. */
void dzChannelRelease(dzChannel *channel);

/* <------------------------------------------------------------ [src/die.c] */

/* Creates a die with the given configuration. */
dzDie *dzDieCreate(dzDieConfig config);

/* Releases the memory allocated for the `die`. */
void dzDieRelease(dzDie *die);

// TODO: dzDieProgramPage

// TODO: dzDieReadPage

/* <---------------------------------------------------------- [src/utils.c] */

/* Returns the next pseudo-random number from the xoshiro256++ generator. */
dzU64 dzUtilsRand(void);

/* ========================================================================> */

#ifdef __cplusplus
}
#endif  // `__cplusplus`

#endif  // `SSDEEZ_H`
