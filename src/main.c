#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <zephyr/sys/printk.h>

#include "ring_buffer.h"
#include "ema.h"

#define HR_MIN_VALUE 44
#define HR_MAX_VALUE 185
#define HR_RANGE     (HR_MAX_VALUE - HR_MIN_VALUE + 1)

static struct ring_buffer hr_ring_buffer;
static ring_buffer_data_t hr_storage[CONFIG_APP_RING_BUFFER_CAPACITY];

/* Protect shared ring buffer access between producer and consumer threads. */
K_MUTEX_DEFINE(hr_buffer_mutex);

/**
 * @brief Generate one simulated heart-rate sample in the range [44, 185].
 */
static int generate_hr_sample(void)
{
    return (int)(HR_MIN_VALUE + (sys_rand32_get() % HR_RANGE));
}

/**
 * @brief Producer thread.
 *
 * Generates one heart-rate sample every second and stores it in the shared
 * ring buffer. When the buffer is full, the oldest sample is removed first so
 * the buffer always contains the latest window of data.
 */
static void producer_thread_cb(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (1) {
        int sample = generate_hr_sample();
        ring_buffer_data_t dropped_sample;
        size_t current_size = 0U;

        k_mutex_lock(&hr_buffer_mutex, K_FOREVER);

        if (ring_buffer_is_full(&hr_ring_buffer)) {
            (void)ring_buffer_get(&hr_ring_buffer, &dropped_sample);
        }

        (void)ring_buffer_put(&hr_ring_buffer, (ring_buffer_data_t)sample);
        current_size = ring_buffer_get_size(&hr_ring_buffer);

        k_mutex_unlock(&hr_buffer_mutex);

        printk("Producer: HR=%d bpm, buffer_size=%zu\n", sample, current_size);

        k_sleep(K_SECONDS(CONFIG_APP_PRODUCER_PERIOD_SECONDS));
    }
}

/**
 * @brief Consumer thread.
 *
 * Every 10 seconds, copies all currently stored samples from the shared ring
 * buffer, computes the EMA from those samples, and reports the result.
 *
 * The EMA calculation is done outside the mutex-protected section so the
 * shared buffer is locked only for the short copy operation.
 */
static void consumer_thread_cb(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    ring_buffer_data_t samples[CONFIG_APP_RING_BUFFER_CAPACITY];
    size_t sample_count = 0U;
    float ema = 0.0f;

    while (1) {
        k_sleep(K_SECONDS(CONFIG_APP_CONSUMER_PERIOD_SECONDS));

        k_mutex_lock(&hr_buffer_mutex, K_FOREVER);
        ring_buffer_status_t status = ring_buffer_drain(&hr_ring_buffer,
                                                        samples,
                                                        CONFIG_APP_RING_BUFFER_CAPACITY,
                                                        &sample_count);
        k_mutex_unlock(&hr_buffer_mutex);

        if (status == RING_BUFFER_ERROR_EMPTY) {
            printk("Consumer: no samples available\n");
            continue;
        }

        if (status != RING_BUFFER_SUCCESS) {
            printk("Consumer: failed to fetch samples\n");
            continue;
        }

        if (!ema_calculation(samples, sample_count, &ema)) {
            printk("Consumer: EMA calculation failed\n");
            continue;
        }

        int ema_x100 = (int)(ema * 100.0f + 0.5f);
        printk("Consumer: fetched=%zu, EMA=%d.%02d bpm\n",
               sample_count,
               ema_x100 / 100,
               ema_x100 % 100);
    }
}

/*
 * Statically create both threads.
 * Delay argument is 0, so both threads start immediately after boot.
 */
K_THREAD_DEFINE(producer_tid,
                CONFIG_APP_THREAD_STACK_SIZE,
                producer_thread_cb,
                NULL, NULL, NULL,
                CONFIG_APP_THREAD_PRIORITY,
                0,
                0);

K_THREAD_DEFINE(consumer_tid,
                CONFIG_APP_THREAD_STACK_SIZE,
                consumer_thread_cb,
                NULL, NULL, NULL,
                CONFIG_APP_THREAD_PRIORITY,
                0,
                0);

int main(void)
{
    ring_buffer_status_t status;

    status = ring_buffer_init(&hr_ring_buffer,
                              hr_storage,
                              CONFIG_APP_RING_BUFFER_CAPACITY);

    if (status != RING_BUFFER_SUCCESS) {
        printk("Failed to initialize ring buffer\n");
        return -1;
    }

    printk("Heart-rate simulation app started\n");
    printk("Buffer capacity: %d\n", CONFIG_APP_RING_BUFFER_CAPACITY);
    printk("Producer period: %d s\n", CONFIG_APP_PRODUCER_PERIOD_SECONDS);
    printk("Consumer period: %d s\n", CONFIG_APP_CONSUMER_PERIOD_SECONDS);

    return 0;
}