# Contributing to NovaOS

NovaOS is developed incrementally. Contributions should be focused, documented, and easy to verify.

Before changing a subsystem, read its design documentation and identify the invariant or user-visible behavior being changed. Keep commits small and explain the reason for the change. New low-level code should include tests, diagnostics, or a clear plan for adding them.

Run `make kernel` and `make smoke` before opening a pull request. Do not commit generated files under `build/`. Security-sensitive changes must include threat-model notes and receive review before merging.
