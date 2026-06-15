# Tested Functionalities

This file lists the unit-tested behaviors covered by the ZTest suites.

## Ring buffer tests (`tests/ring_buffer/src/test_ring_buffer.c`)
- Initialization success and initial state (empty, not full, size zero).
- Initialization failures: NULL ring buffer pointer, NULL storage pointer, and
  zero capacity.
- `ring_buffer_put` success, full-buffer failure, NULL buffer failure.
- `ring_buffer_get` success, empty-buffer failure, NULL buffer failure, NULL
  output pointer failure.
- FIFO ordering, including wraparound behavior.
- `ring_buffer_clear` resets state and allows buffer reuse.
- Size tracking across put/get operations.
- Capacity-one edge case behavior.
- `ring_buffer_drain` success, invalid-argument failure (NULL ring buffer), and
  invalid-capacity failure.
- Query helpers for normal and NULL buffer cases:
  `ring_buffer_is_empty`, `ring_buffer_is_full`, `ring_buffer_get_size`.

## EMA tests (`tests/ema/src/test_ema.c`)
- Failure on NULL sample pointer.
- Failure on NULL output pointer.
- Failure on zero sample size.
- Single-sample EMA equals the sample value.
- Known sequence EMA matches expected result.
- Constant sequence EMA remains constant.
- Mixed signed sequence EMA behaves correctly.

## Smoke integration test (`.github/workflows/smoke-test.yml`)
- Builds and runs the real Zephyr application on `native_sim` in GitHub Actions.
- Runs producer and consumer threads together for a sustained smoke duration.
- Uses `ci/smoke.conf` to keep the ring buffer small and make the producer run
  faster than the consumer.
- Validates the end-to-end application path:
  `producer -> ring buffer -> overflow/drop oldest -> consumer -> EMA -> logs`.
- Confirms that the application starts, producer samples are generated, buffer
  overflow/drop handling is exercised, samples are consumed, and EMA output is
  printed.
- Saves full smoke logs and validation counts as GitHub Actions artifacts.

Detailed FIFO behavior, wraparound behavior, API error handling, and exact EMA
math are covered by the unit tests above.
