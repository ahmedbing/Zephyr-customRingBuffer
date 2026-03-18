#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate the Exponential Moving Average (EMA) of a sample array.
 *
 * This function calculates the EMA over the provided input samples using the
 * standard recursive exponential smoothing formula:
 *     EMA_0 = x_0
 *     EMA_n = alpha * x_n + (1 - alpha) * EMA_(n-1)
 *
 * The first sample is used as the initial EMA value.
 *
 * @param samples Pointer to the input sample array.
 * @param sample_size Number of valid samples in the input array.
 * @param output_ema Pointer to the output variable where the calculated EMA
 *
 * @return true if the EMA was calculated successfully, otherwise false.
 */
bool ema_calculation(const int *samples,
                     size_t sample_size,
                     float *output_ema);

#ifdef __cplusplus
}
#endif