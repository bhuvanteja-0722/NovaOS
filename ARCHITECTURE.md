# NovaOS Architecture

## Product boundary

NovaOS is a general-purpose operating system. The desktop, applications, security services, and optional AI tools are userspace components built on a stable kernel and service boundary.

```text
Desktop and native applications
            │
Window manager, settings, files, shell, search
            │
Application runtime and security broker
            │
System calls, capabilities, and service interfaces
            │
Kernel: memory, processes, scheduler, IPC, VFS, networking, drivers
            │
Hardware and firmware
```

## Phase 0 boot path

```text
Firmware/GRUB
    → Multiboot handoff
    → boot/boot.S
    → bootstrap stack
    → kernel/kmain
    → serial initialization
    → diagnostic success marker
```

The bootstrap is intentionally narrow. It proves that the repository can produce a valid kernel image, package it into an ISO, boot it under QEMU, and expose deterministic diagnostics. It is not yet the final x86-64 kernel architecture.

## Future boundaries

The kernel will expose small, documented interfaces for memory, processes, system calls, virtual filesystems, devices, networking, and security policy. Applications will not directly manipulate hardware. They will request services through userspace or kernel interfaces subject to capability checks.

Security policy should be explicit at boundaries: process identity, address-space ownership, resource handles, filesystem capabilities, network capabilities, device capabilities, and secret-service capabilities.
