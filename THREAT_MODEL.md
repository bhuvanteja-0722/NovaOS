# NovaOS Threat Model

## Scope

The initial threat model covers the kernel, processes, filesystem, application runtime, network policy, credentials, updates, and recovery. Phase 0 only establishes boot and diagnostics; its security properties are intentionally limited.

## Assets

NovaOS must protect system integrity, user files, application boundaries, credentials, authentication material, update provenance, and recovery data.

## Primary threats

| Threat | Desired control |
|---|---|
| Malformed or compromised application | Process isolation, sandbox, capabilities |
| Unauthorized file access | Per-process filesystem capabilities and permission checks |
| Unauthorized network access | Explicit socket policy and firewall broker |
| Credential exfiltration | OS-mediated vault and least-privilege access |
| Malicious or broken update | Signature verification, recovery point, rollback |
| Kernel memory corruption | Page permissions, bounds discipline, testing, review, hardening |
| Hardware or driver failure | Narrow initial hardware target, fault isolation, recovery |

This document is a living specification. Each new subsystem must add assumptions, trust boundaries, abuse cases, and tests before integration.
