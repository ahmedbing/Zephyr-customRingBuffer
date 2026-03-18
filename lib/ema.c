#include "ema.h"

/*
 *   Static Functions
 */

/**
 * @brief Derive the EMA smoothing factor from the number of samples.
 * @param count Number of samples used in the EMA calculation.
 * @return Calculated alpha value, or 0.0f if count is zero.
 */
static float alpha_from_count(const size_t count)
{
    if (count == 0U) {
        return 0.0f;
    }

    return 2.0f / ((float)count + 1.0f);
}

/*
 * EMA API Implementation
 */

bool ema_calculation(const int * samples,
                     size_t sample_size,
                     float * output_ema)
{
    if ((samples == NULL) || (output_ema == NULL) || (sample_size == 0U)) {
        return false;
    }
    float alpha = alpha_from_count(sample_size);
    float ema = (float)samples[0];
    
    for(size_t index = 1U; index < sample_size; ++index)
    {
        ema = (alpha * (float)samples[index]) + ((1.0f - alpha) * ema);
    }
    *output_ema = ema;
    return true;
}