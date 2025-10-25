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

#include "greatest.h"
#include "ssdeez.h"

/* Macros =================================================================> */

// clang-format off

#define DZ_TEST_GAUSSIAN_ITERATION_COUNT   1000
#define DZ_TEST_RANDRANGE_ITERATION_COUNT  1000

// clang-format on

/* Private Function Prototypes ============================================> */

TEST dzTestGaussian(void);
TEST dzTestRandRange(void);

/* Public Functions =======================================================> */

SUITE(dzTestUtils) {
    RUN_TEST(dzTestGaussian);
    RUN_TEST(dzTestRandRange);
}

/* Private Functions ======================================================> */

TEST dzTestGaussian(void) {
    dzF64 samples[DZ_TEST_GAUSSIAN_ITERATION_COUNT], sampleMean = 0.0;

    {
        for (int i = 0; i < DZ_TEST_GAUSSIAN_ITERATION_COUNT; i++) {
            samples[i] = dzUtilsGaussian(0.0, 1.0);

            sampleMean += samples[i];
        }

        sampleMean /= DZ_TEST_GAUSSIAN_ITERATION_COUNT;

        ASSERT_IN_RANGE(0.0, sampleMean, 1.0);
    }

    dzF64 kurtosis = 0.0;

    {
        dzF64 kNumerator = 0.0, kDenominator = kNumerator;

        for (int i = 0; i < DZ_TEST_GAUSSIAN_ITERATION_COUNT; i++) {
            dzF64 deviation = samples[i] - sampleMean;
            dzF64 squaredDeviation = deviation * deviation;

            kDenominator += squaredDeviation;
            kNumerator += squaredDeviation * squaredDeviation;
        }

        kNumerator /= DZ_TEST_GAUSSIAN_ITERATION_COUNT;
        kDenominator /= DZ_TEST_GAUSSIAN_ITERATION_COUNT;

        kurtosis = (kNumerator / (kDenominator * kDenominator));

        ASSERT_IN_RANGE(3.0, kurtosis, 1.0);
    }

    PASS();
}

TEST dzTestRandRange(void) {
    ASSERT_EQ(dzUtilsRandRange(0U, 0U), 0U);
    ASSERT_EQ(dzUtilsRandRange(UINT64_MAX, UINT64_MAX), UINT64_MAX);

    {
        dzU64 minValue = (dzU64) UINT32_MAX;
        dzU64 maxValue = minValue + 1U;

        for (int i = 0; i < DZ_TEST_RANDRANGE_ITERATION_COUNT; i++) {
            ASSERT_GTE(dzUtilsRandRange(minValue, maxValue), minValue);
            ASSERT_LTE(dzUtilsRandRange(maxValue, minValue), maxValue);
        }
    }

    PASS();
}
