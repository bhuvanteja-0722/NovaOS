# Testing NovaOS

NovaOS uses layered tests. A subsystem is not complete when it compiles; it is complete when its documented invariants are exercised and the result is reproducible.

## Phase 0 smoke test

`make smoke` builds the ISO, starts QEMU without a graphical display, captures serial output, and verifies the marker `NOVAOS_PHASE0_BOOT_OK`.

## Planned test layers

| Layer | Purpose |
|---|---|
| Host unit tests | Validate pure algorithms without booting the kernel |
| Kernel tests | Validate memory, interrupts, scheduling, and syscall invariants in a controlled environment |
| Integration tests | Verify interactions among kernel, userspace, storage, and drivers |
| System smoke tests | Boot QEMU and exercise user-visible behavior |
| Security tests | Confirm denied capabilities, isolation, policy enforcement, and recovery behavior |
| Regression tests | Prevent previously fixed defects from returning |

Every bug fix should add a regression test where practical. Tests must not depend on network availability or mutable host state.
