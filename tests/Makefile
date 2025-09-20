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

PROJECT_NAME = ssdeez-tests

LOG_PREFIX = ${_COLOR_BEGIN} ~>${_COLOR_END}

# ============================================================================>

BINARY_PATH = bin
INCLUDE_PATH = include
SOURCE_PATH = src

SSDEEZ_INCLUDE_PATH = ../include
SSDEEZ_LIBRARY_PATH = ../lib

OBJECTS = \
	${SOURCE_PATH}/test_die_ops.o  \
	${SOURCE_PATH}/main.o

TARGET = ${BINARY_PATH}/${PROJECT_NAME}

# ============================================================================>

CC = cc
AR = ar

CFLAGS = -D_DEFAULT_SOURCE -g                   \
	-I${SSDEEZ_INCLUDE_PATH} -I${INCLUDE_PATH}  \
	-O2 -std=c99

# AHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
CFLAGS += -fsanitize=address,leak,undefined -Wall -Werror -Wextra -Wpedantic

LDFLAGS = -L${SSDEEZ_LIBRARY_PATH}
LDLIBS = -lssdeez

# ============================================================================>

all: pre-build build post-build

pre-build:
	@printf "${LOG_PREFIX} CC = ${CC}, AR = ${AR}, MAKE = ${MAKE}\n"

build: ${TARGET}

.c.o:
	@printf "${LOG_PREFIX} Compiling: $@ (from $<)\n"
	@${CC} -c $< -o $@ ${CFLAGS}

${TARGET}: ${OBJECTS}
	@mkdir -p ${BINARY_PATH}
	@${CC} ${OBJECTS} -o $@ ${CFLAGS} ${LDFLAGS} ${LDLIBS}

post-build:
	@printf "${LOG_PREFIX} Build complete.\n"

# ============================================================================>

clean:
	@printf "${LOG_PREFIX} Cleaning up.\n"
	@rm -f ${BINARY_PATH}/${PROJECT_NAME}
	@rm -f ${SOURCE_PATH}/*.o

# ============================================================================>

rebuild: clean all

# ============================================================================>