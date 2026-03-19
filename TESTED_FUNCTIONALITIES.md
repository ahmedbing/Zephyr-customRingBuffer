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
