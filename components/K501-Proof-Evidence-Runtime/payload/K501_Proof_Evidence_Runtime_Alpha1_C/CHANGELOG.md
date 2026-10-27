# Changelog

## 0.1.0-alpha.1 — 2026-10-19

- Initial native C11 Proof/Evidence Runtime candidate.
- Explicit argv execution without an inserted shell.
- Run capture, normalized Evidence Record, SHA-256 manifest, and QH256 v2 self-test.
- Separate-process smoke integration with the existing AIONARC Runtime Alpha 1 binary.
- Execution record distinguishes matching expected exit (`PASS`), unexpected exit (`FAIL`), and incomplete transcript capture (`PARTIAL`).
- No automatic gate acceptance and no QH256 evidence-status mapping.
- Existing `k501-run.sh` and all operator-host/canonical workspace files left untouched.
