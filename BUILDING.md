# Building NovaOS

## Dependencies

The Phase 0 build uses a freestanding 32-bit bootstrap kernel loaded by GRUB and executed in QEMU. The host must provide GCC, binutils, NASM, GRUB utilities, xorriso, and QEMU.

```sh
sudo apt-get update
sudo apt-get install build-essential gcc binutils nasm qemu-system-x86 grub-pc-bin grub-common xorriso
```

## Commands

| Command | Purpose |
|---|---|
| `make kernel` | Compile and validate the Multiboot kernel |
| `make iso` | Build `build/novaos.iso` |
| `make run` | Boot the ISO with serial output attached to the terminal |
| `make smoke` | Boot automatically and verify `NOVAOS_PHASE0_BOOT_OK` |
| `make clean` | Remove generated build files |

The build is intentionally freestanding: it does not link against a host operating-system C library. The initial GRUB handoff is 32-bit; later milestones will establish the x86-64 execution environment before user processes are introduced.

## Debugging

Use `make run` for serial diagnostics. GDB support will be added with the first interrupt and memory-management milestones. Generated files belong under `build/` and must not be committed.
