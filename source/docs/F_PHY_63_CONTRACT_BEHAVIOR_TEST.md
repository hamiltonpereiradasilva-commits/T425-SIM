# F_PHY-63 — Portable CORE <-> Link Service Contract Behavior Test

## Objective

Validate the platform-independent behavior of:

- `source/t425_core/phy/t425_core_phy.h`
- `source/t425_core/phy/t425_core_phy.c`

The test covers callback attachment, endpoint validation, TX dispatch, RX availability, RX retrieval, return-code propagation, and boundary behavior.

## Scope

The test is a host-side C11 unit test using mock callbacks.

It validates only the current portable contract. It does not validate:

- RP2040 hardware;
- PIO;
- DMA;
- GPIO;
- IRQ;
- multicore behavior;
- physical Link0..3 routing;
- physical ACK or retry behavior;
- FIFO behavior;
- rendezvous behavior;
- architectural OUT completion;
- CMake integration;
- the RP2040 PHY implementation.

## Test Location

```text
tests/t425_core/test_f_phy_63_contract.c
```

## Compile and Run

```sh
gcc -std=c11 -Wall -Wextra -Werror -pedantic \
    -Isource/t425_core/phy \
    tests/t425_core/test_f_phy_63_contract.c \
    source/t425_core/phy/t425_core_phy.c \
    -o /tmp/t425_f_phy_63_contract_test \
    && /tmp/t425_f_phy_63_contract_test
```

The generated executable must remain under `/tmp`.

## Required Coverage

### Initialization rejection

Verify rejection of:

- null `phy`;
- null `ops`;
- endpoint count `0`;
- endpoint count greater than `T425_CORE_PHY_MAX_ENDPOINTS`;
- null TX callback;
- null RX-availability callback;
- null RX-get callback.

### Valid initialization

Verify:

- return value is `true`;
- `attached == true`;
- endpoint count is stored;
- all callback pointers are copied;
- callback context is copied.

### TX

Verify:

- valid endpoint dispatches exactly once;
- endpoint ID and frame reach the callback unchanged;
- callback return value is propagated;
- invalid endpoint is rejected without callback invocation;
- frame greater than `0xFF` is rejected without callback invocation;
- null and unattached PHY objects are rejected.

### RX availability

Verify:

- valid endpoint dispatches exactly once;
- callback boolean is propagated;
- invalid endpoint does not dispatch;
- null and unattached PHY objects return `false`.

### RX retrieval

Verify:

- valid endpoint dispatches exactly once;
- endpoint ID reaches the callback unchanged;
- callback-written output is preserved;
- callback return value is propagated;
- invalid endpoint does not dispatch;
- null output pointer is rejected without dispatch;
- null and unattached PHY objects are rejected.

### Endpoint boundaries

With endpoint count `4`:

- endpoint `0` is accepted;
- endpoint `3` is accepted;
- endpoint `4` is rejected.

## Architectural Boundary

The test treats `endpoint_id` as Link Service logical endpoint identity. It does not treat it as:

- `t425_channel_select`;
- physical Link0..3 selection;
- a channel address;
- a rendezvous descriptor;
- an architectural OUT completion token.

## Pass Criteria

The test passes only when:

1. compilation succeeds with all requested warnings treated as errors;
2. the executable exits with return code `0`;
3. the executable prints:

```text
F_PHY-63 contract behavior: PASS
```

## Expected Result

```text
F_PHY-63 CONTRACT BEHAVIOR TEST PASS
```

## Follow-up

This test does not authorize CMake integration, RP2040 PHY validation, architectural channel integration, or F_PHY-64.
