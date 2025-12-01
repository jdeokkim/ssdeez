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

#include <float.h>

#include "greatest.h"
#include "ssdeez.h"

/* Macros =================================================================> */

// clang-format off

#define DZ_TEST_GAUSSIAN_SAMPLE_COUNT   100000
#define DZ_TEST_RANDRANGE_SAMPLE_COUNT  100000

// clang-format on

/* Private Function Prototypes ============================================> */

TEST dzTestGaussian(void);
TEST dzTestRandRange(void);
TEST dzTestReadBitsFromBytes(void);
TEST dzTestInlineFunctions(void);

/* Public Functions =======================================================> */

SUITE(dzTestUtils) {
    RUN_TEST(dzTestGaussian);
    RUN_TEST(dzTestRandRange);
    RUN_TEST(dzTestReadBitsFromBytes);
    RUN_TEST(dzTestInlineFunctions);
}

/* Private Functions ======================================================> */

TEST dzTestGaussian(void) {
    dzF64 samples[DZ_TEST_GAUSSIAN_SAMPLE_COUNT];

    dzF64 sampleMean = 0.0;

    {
        for (int i = 0; i < DZ_TEST_GAUSSIAN_SAMPLE_COUNT; i++) {
            samples[i] = dzUtilsGaussian(0.0, 1.0);

            sampleMean += samples[i];
        }

        sampleMean /= DZ_TEST_GAUSSIAN_SAMPLE_COUNT;

        ASSERT_IN_RANGE(0.0, sampleMean, 1.0);
    }

    {
        dzF64 fourthMoment = 0.0, variance = 0.0;

        for (int i = 0; i < DZ_TEST_GAUSSIAN_SAMPLE_COUNT; i++) {
            dzF64 deviation = samples[i] - sampleMean;
            dzF64 squaredDeviation = deviation * deviation;

            fourthMoment += (squaredDeviation * squaredDeviation);
            variance += squaredDeviation;
        }

        fourthMoment /= DZ_TEST_GAUSSIAN_SAMPLE_COUNT;
        variance /= DZ_TEST_GAUSSIAN_SAMPLE_COUNT;

        /*
            NOTE: We need to make sure our Gaussian distribution
                  looks very similar to the normal distribution!
        */
        dzF64 kurtosis = (fourthMoment / (variance * variance));

        ASSERT_IN_RANGE(3.0, kurtosis, 0.1);
    }

    PASS();
}

TEST dzTestRandRange(void) {
    ASSERT_EQ(dzUtilsRandRange(0U, 0U), 0U);
    ASSERT_EQ(dzUtilsRandRange(UINT64_MAX, UINT64_MAX), UINT64_MAX);

    {
        dzU64 minValue = 0U;
        dzU64 maxValue = UINT32_MAX;

        for (int i = 0; i < DZ_TEST_RANDRANGE_SAMPLE_COUNT; i++) {
            ASSERT_GTE(dzUtilsRandRange(minValue, maxValue), minValue);
            ASSERT_LTE(dzUtilsRandRange(maxValue, minValue), maxValue);
        }
    }

    ASSERT_EQ(dzUtilsRandRangeF64(0.0, 0.0), 0.0);
    ASSERT_EQ(dzUtilsRandRangeF64(DBL_MAX, DBL_MAX), DBL_MAX);

    {
        dzF64 minValue = -FLT_MAX;
        dzF64 maxValue = FLT_MAX;

        for (int i = 0; i < DZ_TEST_RANDRANGE_SAMPLE_COUNT; i++) {
            ASSERT_GT(dzUtilsRandRangeF64(minValue, maxValue), minValue);
            ASSERT_LT(dzUtilsRandRangeF64(maxValue, minValue), maxValue);
        }
    }

    PASS();
}

TEST dzTestReadBitsFromBytes(void) {
    dzByte bytes[] = { 0x12, 0x34, 0x56, 0x78 };

    dzByteArray byteArray = { .ptr = bytes,
                              .size = sizeof bytes / sizeof *bytes };

    dzU64 value = UINT64_MAX;

    {
        dzUtilsReadBitsFromBytes(byteArray, 0U, 1U, NULL);

        ASSERT_EQ(value, UINT64_MAX);

        dzUtilsReadBitsFromBytes(byteArray, 0U, 0U, &value);

        ASSERT_EQ(value, UINT64_MAX);

        dzUtilsReadBitsFromBytes(byteArray, 31U, 0U, &value);

        ASSERT_EQ(value, UINT64_MAX);

        dzUtilsReadBitsFromBytes(byteArray, 31U, 1U, &value);

        ASSERT_EQ(value, 0x00U);
    }

    {
        dzUtilsReadBitsFromBytes(byteArray, 0U, 4U, &value);

        ASSERT_EQ(value, 0x01U);

        dzUtilsReadBitsFromBytes(byteArray, 0U, 7U, &value);

        ASSERT_EQ(value, 0x09U);

        dzUtilsReadBitsFromBytes(byteArray, 0U, 8U, &value);

        ASSERT_EQ(value, 0x12U);

        dzUtilsReadBitsFromBytes(byteArray, 0U, 10U, &value);

        ASSERT_EQ(value, 0x48U);
    }

    {
        dzUtilsReadBitsFromBytes(byteArray, 10U, 4U, &value);

        ASSERT_EQ(value, 0x0DU);

        dzUtilsReadBitsFromBytes(byteArray, 10U, 8U, &value);

        ASSERT_EQ(value, 0xD1U);

        dzUtilsReadBitsFromBytes(byteArray, 10U, 9U, &value);

        ASSERT_EQ(value, 0x01A2U);

        dzUtilsReadBitsFromBytes(byteArray, 10U, 13U, &value);

        ASSERT_EQ(value, 0x1A2BU);
    }

    PASS();
}

TEST dzTestInlineFunctions(void) {
    ASSERT_EQ(dzUtilsClampF64(0.0, 0.0, 0.0), 0.0);
    ASSERT_EQ(dzUtilsClampF64(-0.5, 0.0, 1.0), 0.0);
    ASSERT_EQ(dzUtilsClampF64(1.5, 0.0, 1.0), 1.0);
    ASSERT_EQ(dzUtilsClampF64(1.5, 1.0, 0.0), 1.0);

    PASS();
}
