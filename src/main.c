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

#include "external/json.h"

#define OPTPARSE_IMPLEMENTATION
#include "external/optparse.h"

#include "ssdeez.h"

/* Private Function Prototypes ============================================> */

/* Shows the 'usage' message and terminates this program. */
static void dzMainShowUsage(char *programName, char *errorMessage);

/* Public Functions =======================================================> */

int main(int argc, char *argv[]) {
    DZ_API_UNUSED_VARIABLE(argc);

    struct optparse options;

    optparse_init(&options, argv);

    // const char *configPath = NULL;
    // const char *tracePath = NULL;

    {
        int option = -1;

        while ((option = optparse(&options, "c:t:")) != -1) {
            switch (option) {
                case 'c':
                    // configPath = options.optarg;

                    break;

                case 't':
                    // tracePath = options.optarg;

                    break;

                case '?':
                    dzMainShowUsage(argv[0], options.errmsg);

                    break;
            }
        }

        if (option < 0) dzMainShowUsage(argv[0], NULL);
    }

    // TODO: ...

    return 0;
}

/* Private Functions ======================================================> */

/* Shows the 'usage' message and terminates this program. */
static void dzMainShowUsage(char *programName, char *errorMessage) {
    if (programName == NULL) programName = "ssdeez";

    if (errorMessage != NULL)
        (void) fprintf(stderr, "%s: %s\n\n", programName, errorMessage);

    // clang-format off

    (void) fprintf(
        stderr,
        "Usage: %s -c config [-t trace_file]\n"
        "\n"
        "Options:\n"
        "  -c config    Specify the path to the configuration file\n"
        "  -t trace     Specify the path to the workload trace file\n"
        "\n"
        "SSDeez v" DZ_API_VERSION 
        " (https://github.com/jdeokkim/ssdeez)\n",
        programName
    );

    // clang-format on

    exit(EXIT_FAILURE);
}
