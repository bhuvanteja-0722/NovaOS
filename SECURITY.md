# NovaOS Security

Security is a foundational property of NovaOS and should remain unobtrusive during normal use. The intended architecture uses defense in depth across boot, kernel, applications, networking, and data.

## Planned security layers

| Layer | Planned control |
|---|---|
| Boot | Verified boot chain and signed system components |
| Kernel | User/kernel separation, virtual memory permissions, process isolation, protected syscalls |
| Applications | Sandboxing, capabilities, explicit filesystem/device/network permissions |
| Network | Per-application firewall policy and controlled sockets |
| Data | Encrypted storage, separated system/user data, backups, recovery, rollback |
| Credentials | Nova Vault mediated by permission checks rather than ambient environment variables |

No custom cryptographic algorithms will be designed. Security-sensitive implementation requires documented assumptions, tests, review, and preferably established primitives or trusted libraries.

## Reporting

Until a public security process exists, report suspected vulnerabilities privately to the project owner rather than publishing exploit details in an issue. Reproduction steps, affected commit, impact, and mitigation ideas are useful.
