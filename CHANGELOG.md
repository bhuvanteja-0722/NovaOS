# Changelog

All notable changes to NovaOS will be documented here.

## Unreleased

### Added

- M1 kernel architecture layer with GDT, IDT, PIC remapping, PIT timer interrupts, and exception stubs.
- M2 bootstrap physical-memory allocator using Multiboot memory information.
- M3 process table and syscall dispatcher scaffolding with kernel self-tests.
- M4 userspace ABI, executable metadata validation, scheduler-ready process context, and software syscall entry validation.
- M5 bounded in-memory VFS with safe absolute paths, directories, files, bounded reads/writes, and kernel self-tests.
- M6 ATA PIO block-device foundation with deterministic QEMU disk image creation and sector read/write validation.
- M7 versioned persistent filesystem superblock, metadata validation, QEMU image formatter, and mounted-storage smoke test.
- M8 checksummed persistent VFS nodes, mounted `/etc/motd` content, invalid-path rejection, and persistent node-table validation.
- M9 bounded storage syscalls for `open`, `read`, and `close`, per-process-style descriptors, and stable negative error codes.
- M10 scheduler-ready round-robin queue with timer-driven tick integration and rotation self-test.
- M10 page-range address-space metadata, process-specific address-space ownership, guarded process termination, overflow-safe user-pointer validation, guarded user-mode selector readiness contract, corrected ring-3 GDT code/data descriptors, fail-closed transition guard until a complete user process is ready, validated identity page-table layout, and controlled CR3/CR0 paging activation.
- M11 fixed built-in user-probe image copied into the identity-mapped user page, aligned the user stack top with the mapped `0x00800000` stack page, added a dedicated guarded iret trampoline, installed a ring-0 TSS with a dedicated interrupt stack, added validated syscall-frame capture, a one-shot probe-exit decision, and a kernel-owned non-returning termination trampoline, validated user entry/stack context, process-specific ownership, guarded termination, protected probe mapping, validated ring-3 iret frame, and fail-closed transition checks; ring-3 entry remains disabled until a complete return/termination cycle is tested.
- Phase 0 repository structure.
- Freestanding Multiboot kernel bootstrap.
- GRUB ISO packaging and QEMU run target.
- Serial kernel diagnostics and automated boot smoke test.
- Initial architecture, security, threat model, testing, and roadmap documentation.
