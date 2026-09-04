# Changelog

All notable changes to NovaOS will be documented here.

## Unreleased

### Added

- M1 kernel architecture layer with GDT, IDT, PIC remapping, PIT timer interrupts, and exception stubs.
- M2 bootstrap physical-memory allocator using Multiboot memory information.
- M3 process table and syscall dispatcher scaffolding with kernel self-tests.
- M4 userspace ABI, executable metadata validation, scheduler-ready process context, and software syscall entry validation.
- M5 bounded in-memory VFS with safe absolute paths, directories, files, bounded reads/writes, and kernel self-tests.
- Phase 0 repository structure.
- Freestanding Multiboot kernel bootstrap.
- GRUB ISO packaging and QEMU run target.
- Serial kernel diagnostics and automated boot smoke test.
- Initial architecture, security, threat model, testing, and roadmap documentation.
