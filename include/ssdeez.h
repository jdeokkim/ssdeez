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
#endif // `__cplusplus`

/* Includes ==============================================================> */

#include <stdlib.h>

/* Macros ================================================================> */

/* Typedefs ==============================================================> */

/* An enumeration that represents the type of a NAND flash cell. */
typedef enum dzCellType_ {
    DZ_CELL_UNKNOWN,
    DZ_CELL_SLC,      // 1 voltage state
    DZ_CELL_MLC,      // 2 voltage states
    DZ_CELL_TLC,      // 3 voltage states
    DZ_CELL_QLC       // 4 voltage states
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
    int planeCountPerDie;
    int blockCountPerPlane;
    int pageCountPerBlock;
    int pageSizeInBytes;
} dzDieConfig;

/* Public Functions =======================================================> */

/* <-------------------------------------------------------- [src/channel.c] */

/* Creates a channel with the given `id`. */
dzChannel *dzChannelCreate(int id);

/* Releases the memory allocated for the `channel`. */
void dzChannelRelease(dzChannel *channel);

/* <------------------------------------------------------------ [src/die.c] */

/* Creates a die with the given configuration. */
dzDie *dzDieCreate(dzDieConfig config);

/* Releases the memory allocated for the `die`. */
void dzDieRelease(dzDie *die);

/* ========================================================================> */

#ifdef __cplusplus
}
#endif  // `__cplusplus`

#endif  // `SSDEEZ_H`
