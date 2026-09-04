# NovaOS Roadmap

Each milestone requires a reproducible acceptance test and a written review before the next milestone begins.

## Completed foundation

### M0 — Repository and build foundation

Build system, freestanding cross-compilation discipline, QEMU, Git, GRUB boot image, serial diagnostics, documentation, CI, and smoke tests.

### M1 — Kernel architecture

GDT, IDT, exception stubs, PIC remapping, PIT timer interrupts, and structured kernel diagnostics.

### M2 — Bootstrap memory

Multiboot memory discovery, page-aligned bootstrap allocation, and memory diagnostics.

### M3 — Process foundation

Process table, PID allocation, current-process tracking, and syscall dispatcher scaffolding.

### M4 — Userspace ABI foundation

Executable metadata validation, scheduler-ready process context, user-stack constants, and software syscall entry validation.

### M5 — In-memory VFS

Safe absolute paths, directories, files, bounded reads/writes, and kernel VFS self-tests.

### M6 — Block storage

ATA PIO sector reads and writes, deterministic QEMU disk-image creation, and storage round-trip validation.

### M7 — Persistent filesystem mount

Versioned superblock, metadata validation, checksummed format fields, QEMU image formatter, and mounted-storage validation.

### M8 — Persistent VFS nodes

Checksummed on-disk node records, persistent `/etc/motd` content, mounted path lookup, invalid-path rejection, and persistent node-table validation.

## Next core milestones

### M9 — User-facing storage syscalls

Per-process file descriptors, validated `open`, `read`, `write`, `close`, and directory operations, with stable error codes, ownership checks, and capability boundaries.

### M10 — User mode and scheduler

User-pointer validation, ring-3 transition, scheduler context switching, process address spaces, timer-driven scheduling, and a real `init` entry path.

### M11 — Shell and core utilities

Userspace shell, executable loading from persistent storage, process lifecycle commands, and core file utilities.

### M12 — Persistent filesystem mutation

On-disk file creation, directory updates, free-space tracking, atomic metadata updates, checksums, and reboot persistence tests.

## Later product milestones

### M13 — Hardware and networking

Keyboard, display, Ethernet, sockets, DNS, and a conservative initial driver set in QEMU.

### M14 — Desktop shell

Window manager, compositor, launcher, settings, file manager, terminal, notifications, accessibility, and multi-monitor support.

### M15 — Applications and security services

Application manifests, package manager, sandbox permissions, capabilities, firewall policy, Nova Vault, signed updates, recovery, rollback, and backups.

### M16 — Ecosystem and polish

SDK, developer tools, documentation refinement, performance, themes, animations, and portfolio demonstrations.
