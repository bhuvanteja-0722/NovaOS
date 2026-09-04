# NovaOS

**NovaOS** is a general-purpose, local-first x86-64 operating system project with a human-designed native interface, defense-in-depth security, application isolation, protected credentials, and reliable recovery.

The project is being developed in small, testable increments. The first milestone is a reproducible bootable kernel that runs in QEMU and reports its initialization state over a serial console.

## Current status

| Area | Status |
|---|---|
| Repository and documentation | In progress |
| Reproducible build | Phase 0 |
| QEMU boot image | Phase 0 |
| Kernel serial diagnostics | Phase 0 |
| Memory management | Planned |
| Processes and user mode | Planned |
| Filesystem and shell | Planned |
| Desktop and applications | Planned |
| Sandboxing and capabilities | Planned |

## Build and test

On Ubuntu or Debian, install the required packages:

```sh
sudo apt-get install build-essential gcc binutils nasm qemu-system-x86 grub-pc-bin grub-common xorriso
```

Build the bootable ISO and run the smoke test:

```sh
make iso
make smoke
```

Run NovaOS interactively with serial output in the terminal:

```sh
make run
```

## Engineering principles

NovaOS is a normal operating system first. Security is a foundational property rather than the product identity. AI is optional and, if included later, will run as an ordinary sandboxed application with explicit capabilities.

Every subsystem must have a design, implementation, tests, documentation, threat analysis, and review. Kernel, memory, filesystem, drivers, networking, authentication, isolation, and cryptography code must not be trusted solely because it compiles.

## Repository map

```text
boot/          Boot assembly and GRUB configuration
kernel/        Kernel implementation
drivers/       Hardware drivers
filesystem/    Filesystem and VFS
networking/    Network stack and policy
security/      Capabilities, permissions, sandboxing, vault
userspace/     Init, shell, and core utilities
desktop/       Display server, window manager, and desktop
apps/          Native applications and package metadata
recovery/      Recovery, snapshots, rollback, and backups
tests/         Unit, integration, and system tests
tools/         Build and developer tooling
docs/          Technical documentation
```

See [BUILDING.md](BUILDING.md), [ARCHITECTURE.md](ARCHITECTURE.md), [SECURITY_AUDIT.md](SECURITY_AUDIT.md), and [ROADMAP.md](ROADMAP.md) for more detail.
