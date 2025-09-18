#
# Copyright (c) 2025 Jaedeok Kim (jdeokkim@protonmail.com)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#

# ============================================================================>

.POSIX:

# ============================================================================>

.PHONY: all clean library rebuild

# ============================================================================>

_COLOR_BEGIN = \033[1;38;5;180m
_COLOR_END = \033[m

# ============================================================================>

PROJECT_NAME = ssdeez

LOG_PREFIX = ${_COLOR_BEGIN} ~>${_COLOR_END}

# ============================================================================>

BINARY_PATH = bin
INCLUDE_PATH = include
LIBRARY_PATH = lib
SOURCE_PATH = src

OBJECTS = \
	${SOURCE_PATH}/channel.o  \
	${SOURCE_PATH}/die.o

TARGET_BIN = ${BINARY_PATH}/${PROJECT_NAME}
TARGET_LIB = ${LIBRARY_PATH}/lib${PROJECT_NAME}.a

# ============================================================================>

CC = cc
AR = ar

CFLAGS = -D_DEFAULT_SOURCE -g -I${INCLUDE_PATH} -O2 -std=c99

# AHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
CFLAGS += -fsanitize=address,leak,undefined -Wall -Werror -Wextra -Wpedantic

LDFLAGS = -L${LIBRARY_PATH}
LDLIBS = -llib${PROJECT_NAME}

# ============================================================================>

all: pre-build build-bin post-build

pre-build:
	@printf "${LOG_PREFIX} CC = ${CC}, AR = ${AR}, MAKE = ${MAKE}\n"

build-bin: ${TARGET_BIN}

.c.o:
	@printf "${LOG_PREFIX} Compiling: $@ (from $<)\n"
	@${CC} -c $< -o $@ ${CFLAGS}

${TARGET_BIN}: ${OBJECTS} ${SOURCE_PATH}/main.o
	@mkdir -p ${BINARY_PATH}
	@${CC} ${OBJECTS} ${SOURCE_PATH}/main.o -o $@ ${CFLAGS}

post-build:
	@printf "${LOG_PREFIX} Build complete.\n"

# ============================================================================>

library: pre-build build-lib post-build

build-lib: ${TARGET_LIB}

${TARGET_LIB}: ${OBJECTS}
	@mkdir -p ${LIBRARY_PATH}
	@printf "${LOG_PREFIX} Linking: ${TARGET_LIB}\n"
	@${AR} rcs ${TARGET_LIB} ${OBJECTS}

# ============================================================================>

clean:
	@printf "${LOG_PREFIX} Cleaning up.\n"
	@rm -f ${BINARY_PATH}/${PROJECT_NAME}
	@rm -f ${LIBRARY_PATH}/*.a
	@rm -f ${SOURCE_PATH}/*.o

# ============================================================================>

rebuild: clean all

# ============================================================================>