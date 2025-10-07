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

#include "greatest.h"
#include "ssdeez.h"

/* Macros =================================================================> */

#define DZ_TEST_GAUSSIAN_MAX_COUNT 1000

/* Private Function Prototypes ============================================> */

TEST dzTestGaussian(void);

/* Public Functions =======================================================> */

SUITE(dzTestUtils) {
    RUN_TEST(dzTestGaussian);
}

/* Private Functions ======================================================> */

TEST dzTestGaussian(void) {
    dzF64 samples[DZ_TEST_GAUSSIAN_MAX_COUNT], sampleMean = 0.0;

    {
        for (int i = 0; i < DZ_TEST_GAUSSIAN_MAX_COUNT; i++) {
            samples[i] = dzUtilsGaussian(0.0, 1.0);

            sampleMean += samples[i];
        }

        sampleMean /= DZ_TEST_GAUSSIAN_MAX_COUNT;

        ASSERT_IN_RANGE(0.0, sampleMean, 1.0);
    }

    dzF64 kurtosis = 0.0;

    {
        dzF64 kNumerator = 0.0, kDenominator = kNumerator;

        for (int i = 0; i < DZ_TEST_GAUSSIAN_MAX_COUNT; i++) {
            dzF64 deviation = samples[i] - sampleMean;
            dzF64 squaredDeviation = deviation * deviation;

            kDenominator += squaredDeviation;
            kNumerator += squaredDeviation * squaredDeviation;
        }

        kNumerator /= DZ_TEST_GAUSSIAN_MAX_COUNT;
        kDenominator /= DZ_TEST_GAUSSIAN_MAX_COUNT;

        kurtosis = (kNumerator / (kDenominator * kDenominator));

        ASSERT_IN_RANGE(3.0, kurtosis, 1.0);
    }

    PASS();
}
