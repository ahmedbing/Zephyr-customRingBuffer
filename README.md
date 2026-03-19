# RingBuffer - Zephyr Ring Buffer + EMA

## Overview
This project implements a circular buffer in C and uses it in a small Zephyr
application that simulates a wearable device. A random number generator acts as
a heart-rate (HR) sensor: in producer thread it produces a new sample every second, stores it in
the ring buffer, and a consumer thread computes an Exponential Moving Average
(EMA) from the buffered samples every 10 seconds.

## Project Layout
- `include/ring_buffer.h` - Ring buffer API.
- `lib/ring_buffer.c` - Ring buffer implementation.
- `include/ema.h` - EMA API.
- `lib/ema.c` - EMA implementation.
- `src/main.c` - Zephyr application (producer + consumer threads).
- `tests/ring_buffer/` - Ring buffer ZTest unit tests.
- `tests/ema/` - EMA ZTest unit tests.
- `Kconfig` - Application configuration options.
- `prj.conf` - Default build configuration.

## Build and Run (Zephyr application)
Prerequisites: Zephyr environment initialized, `west` installed, and a Zephyr
SDK/toolchain available (for example via `ZEPHYR_BASE` and the usual Zephyr
environment scripts).
Tested on: 
- SDK:               nRF Connect SDK v3.2.0
- Toolchain:         nRF Connect SDK Toolchain v3.2.0

Build and run on the native simulator:
```sh
west build -b native_sim -p always .
./build/RingBuffer/zephyr/zephyr.exe
```

## Unit Tests
Ring buffer tests:
```sh
west build -b native_sim -p always tests/ring_buffer
./build/ring_buffer/zephyr/zephyr.exe
```

EMA tests:
```sh
west build -b native_sim/native/64 -p always tests/ema
./build/ema/zephyr/zephyr.exe
```

For a full list of tested functionalities, see `TESTED_FUNCTIONALITIES.md`.

## API Usage Examples
Ring buffer:
```c
#include "ring_buffer.h"

#define CAPACITY 8

static ring_buffer_data_t storage[CAPACITY];
static struct ring_buffer rb;

void example_ring_buffer(void)
{
    ring_buffer_data_t value = 0;
    ring_buffer_init(&rb, storage, CAPACITY);

    ring_buffer_put(&rb, 123);
    ring_buffer_put(&rb, 456);

    ring_buffer_get(&rb, &value);
    /* value == 123 */
}
```

EMA:
```c
#include "ema.h"

void example_ema(void)
{
    const int samples[] = {60, 70, 80};
    float ema = 0.0f;

    if (ema_calculation(samples, 3U, &ema)) {
        /* ema == 72.5 */
    }
}
```

## Configuration
The main application uses the following Kconfig options (see `Kconfig` and
`prj.conf` for defaults):
- `CONFIG_APP_RING_BUFFER_CAPACITY` - Ring buffer capacity.
- `CONFIG_APP_PRODUCER_PERIOD_SECONDS` - HR sample period.
- `CONFIG_APP_CONSUMER_PERIOD_SECONDS` - EMA calculation period.
- `CONFIG_APP_THREAD_STACK_SIZE` - Thread stack size.
- `CONFIG_APP_THREAD_PRIORITY` - Thread priority.

## Requirements Traceability
Functional requirements:
1. Circular Buffer: Implement a circular buffer with a fixed size (size is a
   `KConfig` option via `CONFIG_APP_RING_BUFFER_CAPACITY`).
2. Unit Tests: ZTest unit tests for ring buffer and EMA (`tests/ring_buffer`,
   `tests/ema`).
3. Producer Thread: Random HR generator (44 to 185) every second stored in the
   buffer (`src/main.c`).
4. Consumer Thread: Every 10 seconds, drain all samples in the buffer and
   calculate the EMA (`src/main.c` and `lib/ema.c`).

## Accepted Design Decisions
- The ring buffer uses caller-provided storage to avoid dynamic allocation in
  embedded contexts.
- `ring_buffer_put` does not overwrite data when full; the application chooses
  to drop the oldest sample before inserting a new one.
- The consumer uses `ring_buffer_drain` to collect all samples and then performs
  EMA calculation outside the mutex to minimize lock hold time.
- EMA uses `alpha = 2 / (N + 1)` with the first sample as the initial EMA.
- For the EMA alpha, I used `α = 2 / (N + 1)`, where `N` is the equivalent **N-sample moving average** window. This is a simple and widely used approximation to the more exact exponential form `α = 1 - e^(-Δt / τ)`
- Ring buffer data type is `int` to match the HR sample range and simplify the
  app logic.

## Assumptions
- The Zephyr environment is configured and `west` is available.
- Both thread have same priority level
- Heart-rate values are simulated in the range [44, 185].
- Expected funtionality of consumer to drain all available samples every period to compute the EMA
  over the latest buffered window.

## Improvements / Future Work
- Add integration tests that run the producer/consumer threads together.
- Create pipeline to check each commit with build checks/smoke tests 
- Add a dedicated "overwrite oldest" mode argument in the ring buffer API so the policy
  does not live only in the implementation.
- Threads can be moved to app_threads header/source files.
- Using arm dsp library IIR/biquad functions for optimized float calculation.
- Logger functions in more detailed information at fault detections
- Consider stricter error codes for `ring_buffer_init` to distinguish invalid
  arguments from other failures.
- Implementation of lock-free ring buffer or Zephyr ring buffer alternatives for higher throughput.

## Tested Functionalities
See `TESTED_FUNCTIONALITIES.md`.
