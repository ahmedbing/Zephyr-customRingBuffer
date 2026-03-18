#include <zephyr/ztest.h>
#include <math.h>
#include <ema.h>

#define EMA_FLOAT_TOLERANCE 0.001f

/**
 * @brief Helper to compare two floating-point values within tolerance.
 */
static void zassert_float_close(float actual, float expected, float tolerance, const char *msg)
{
    zassert_true(fabsf(actual - expected) <= tolerance, msg);
}

/**
 * @brief Verify that EMA calculation fails with a NULL sample pointer.
 */
ZTEST(ema, test_ema_fails_with_null_samples)
{
    float output = 0.0f;

    zassert_false(ema_calculation(NULL, 3U, &output),
                  "EMA calculation should fail with NULL samples pointer");
}

/**
 * @brief Verify that EMA calculation fails with a NULL output pointer.
 */
ZTEST(ema, test_ema_fails_with_null_output)
{
    const int samples[] = {10, 20, 30};

    zassert_false(ema_calculation(samples, 3U, NULL),
                  "EMA calculation should fail with NULL output pointer");
}

/**
 * @brief Verify that EMA calculation fails with zero sample size.
 */
ZTEST(ema, test_ema_fails_with_zero_sample_size)
{
    const int samples[] = {10, 20, 30};
    float output = 0.0f;

    zassert_false(ema_calculation(samples, 0U, &output),
                  "EMA calculation should fail when sample size is zero");
}

/**
 * @brief Verify that a single sample returns the same value as the EMA.
 *
 * For a single sample:
 * - alpha = 2 / (1 + 1) = 1.0
 * - EMA = first sample
 */
ZTEST(ema, test_ema_single_sample)
{
    const int samples[] = {75};
    float output = 0.0f;

    zassert_true(ema_calculation(samples, 1U, &output),
                 "EMA calculation should succeed for one sample");

    zassert_float_close(output, 75.0f, EMA_FLOAT_TOLERANCE,
                        "EMA should equal the single input sample");
}

/**
 * @brief Verify EMA against a known sample sequence.
 *
 * For samples [60, 70, 80]:
 * - N = 3
 * - alpha = 2 / (3 + 1) = 0.5
 *
 * EMA_0 = 60
 * EMA_1 = 0.5 * 70 + 0.5 * 60 = 65
 * EMA_2 = 0.5 * 80 + 0.5 * 65 = 72.5
 */
ZTEST(ema, test_ema_known_sequence)
{
    const int samples[] = {60, 70, 80};
    float output = 0.0f;

    zassert_true(ema_calculation(samples, 3U, &output),
                 "EMA calculation should succeed for valid input");

    zassert_float_close(output, 72.5f, EMA_FLOAT_TOLERANCE,
                        "EMA result does not match expected value");
}

/**
 * @brief Verify that a constant input sequence produces the same constant EMA.
 *
 * For repeated identical values, the EMA should remain equal to that value.
 */
ZTEST(ema, test_ema_constant_sequence)
{
    const int samples[] = {88, 88, 88, 88, 88};
    float output = 0.0f;

    zassert_true(ema_calculation(samples, 5U, &output),
                 "EMA calculation should succeed for constant input");

    zassert_float_close(output, 88.0f, EMA_FLOAT_TOLERANCE,
                        "EMA of a constant sequence should remain constant");
}

/**
 * @brief Verify EMA behavior with negative and positive input values.
 *
 * Although the heart-rate use case is positive-only, the function accepts int
 * samples and should still behave correctly with generic signed input.
 */
ZTEST(ema, test_ema_mixed_signed_sequence)
{
    const int samples[] = {-10, 0, 10};
    float output = 0.0f;

    zassert_true(ema_calculation(samples, 3U, &output),
                 "EMA calculation should succeed for signed input values");

    /*
     * N = 3 => alpha = 0.5
     * EMA_0 = -10
     * EMA_1 = 0.5 * 0  + 0.5 * (-10) = -5
     * EMA_2 = 0.5 * 10 + 0.5 * (-5)  = 2.5
     */
    zassert_float_close(output, 2.5f, EMA_FLOAT_TOLERANCE,
                        "EMA result for signed sequence is incorrect");
}

ZTEST_SUITE(ema, NULL, NULL, NULL, NULL, NULL);