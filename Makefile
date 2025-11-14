#
# Copyright (c) 2025 Jaedeok Kim <jdeokkim@protonmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
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
	${SOURCE_PATH}/block.o  \
	${SOURCE_PATH}/chip.o   \
	${SOURCE_PATH}/die.o    \
	${SOURCE_PATH}/onfi.o   \
	${SOURCE_PATH}/page.o   \
	${SOURCE_PATH}/plane.o  \
	${SOURCE_PATH}/utils.o

TARGET_BIN = ${BINARY_PATH}/${PROJECT_NAME}
TARGET_LIB = ${LIBRARY_PATH}/lib${PROJECT_NAME}.a

# ============================================================================>

CC = cc
AR = ar

CFLAGS = -D_DEFAULT_SOURCE -g -I${INCLUDE_PATH} -O2 -std=c99 \
	-fsanitize=address,leak,undefined -Wall -Wconversion \
	-Wdouble-promotion -Werror -Wextra -Wpedantic

LDLIBS = -lm

# ============================================================================>

all: pre-build build-bin build-lib post-build

# ============================================================================>

pre-build:
	@printf "${LOG_PREFIX} CC = ${CC}, AR = ${AR}, MAKE = ${MAKE}\n"

# ============================================================================>

.c.o:
	@printf "${LOG_PREFIX} Compiling: $@ (from $<)\n"
	@${CC} -c $< -o $@ ${CFLAGS}

# ============================================================================>

build-bin: ${TARGET_BIN}

${TARGET_BIN}: ${OBJECTS} ${SOURCE_PATH}/main.o
	@mkdir -p ${BINARY_PATH}
	@${CC} ${OBJECTS} ${SOURCE_PATH}/main.o -o $@ ${CFLAGS} ${LDLIBS}

# ============================================================================>

build-lib: ${TARGET_LIB}

${TARGET_LIB}: ${OBJECTS}
	@mkdir -p ${LIBRARY_PATH}
	@printf "${LOG_PREFIX} Linking: ${TARGET_LIB}\n"
	@${AR} -rcs ${TARGET_LIB} ${OBJECTS}

# ============================================================================>

post-build:
	@printf "${LOG_PREFIX} Build complete.\n"

# ============================================================================>

clean:
	@printf "${LOG_PREFIX} Cleaning up.\n"
	@rm -f ${BINARY_PATH}/${PROJECT_NAME}
	@rm -f ${LIBRARY_PATH}/*.a
	@rm -f ${SOURCE_PATH}/*.o

# ============================================================================>

rebuild: clean all

# ============================================================================>
