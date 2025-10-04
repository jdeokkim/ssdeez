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

/* 
    A structure that represents an interface between the controller 
    and a group of dies. 
*/
struct dzChannel_ {
    dzU64 id;  // The ID of the channel.
    // TODO: ...
};

/* Private Function Prototypes ============================================> */

// TODO: ...

/* Constants ==============================================================> */

// TODO: ...

/* Private Variables ======================================================> */

// TODO: ...

/* Public Functions =======================================================> */

/* Creates a channel with the given `id`. */
dzChannel *dzChannelCreate(dzU64 id) {
    dzChannel *channel = malloc(sizeof *channel);

    if (channel == NULL) return channel;

    channel->id = id;

    return channel;
}

/* Releases the memory allocated for the `channel`. */
void dzChannelRelease(dzChannel *channel) {
    free(channel);
}
