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

#include <stdio.h>
#include <stdlib.h>

#define OPTPARSE_IMPLEMENTATION
#include "external/optparse.h"

#include "ssdeez.h"

/* Private Function Prototypes ============================================> */

static void dzMainShowUsage(char *programName, char *errorMessage);

/* Public Functions =======================================================> */

int main(int argc, char *argv[]) {
    DZ_API_UNUSED_VARIABLE(argc);

    struct optparse options;

    optparse_init(&options, argv);

    int option = -1;

    while ((option = optparse(&options, "c:p:t:")) != -1) {
        switch (option) {
            case 'c':
                // TODO: ...

                break;

            case 'p':
                // TODO: ...
                
                break;

            case 't':
                // TODO: ...
                
                break;

            case '?':
                dzMainShowUsage(argv[0], options.errmsg);

                break;
        }
    }

    // TODO: ...

    return 0;
}

/* Private Functions ======================================================> */

static void dzMainShowUsage(char *programName, char *errorMessage) {
    // clang-format off

    (void) fprintf(
        stderr, 
        "%s: %s\n"
        "\n"
        "Usage: %s -c config_file [-p preset | -t trace_file]\n"
        "\n"
        "Options:\n"
        "  -c config_file   Specify the path to the SSD configuration file\n"
        "  -p preset        Specify which pre-defined workload definition to use\n"
        "  -t trace_file    Specify the path to the workload trace file\n",
        programName, errorMessage, 
        programName
    );

    // clang-format on

    exit(EXIT_FAILURE);
}
