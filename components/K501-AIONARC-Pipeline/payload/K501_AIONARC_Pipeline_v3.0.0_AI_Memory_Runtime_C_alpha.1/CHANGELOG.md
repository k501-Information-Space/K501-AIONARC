# Changelog

## 3.0.0-alpha.1 — engineering candidate

- Introduces `K501-IS-MEMORY-CONTRACT-1.0` and mapping profile `QH256-K501-AIONARC-MEMORY-V3A1`.
- Distinguishes `INVALID_UTF8` from `EMBEDDED_NUL` and records PDF child success separately from text admission.
- Reconstructs validated `text_status` from the index before recomputing per-object QH256 during verification.
- Generates the final index only after candidate state/hash/order and text roundtrip checks, and verifies Base64 path roundtrips before asserting mapping cells 27–30.
- Fixes zero-byte text `TEXT.tsv` parsing by retaining an empty final Base64 field; supports `CAPTURED_EMPTY` as an actual bound text row.
- Adds known-vector SHA-256 tests and mapping-status tests, plus end-to-end invalid UTF-8, embedded NUL, empty-text, file-limit and optional PDF regression cases.
- Uses distinct V3A1 store/frame/text/index/proof schema IDs and a V3A1 frame-chain hash domain. Keeps the QH256 v2 core algebra/wire serialization and prior object-ID/object-record formulas unchanged.
- Enforces generic x86-64 baseline flags in the Makefile command ordering and documents the qualified ABI boundary.
- Replaces the inherited SHA-256 source body with a clean FIPS 180-4 algorithm implementation and known-vector self-tests.
- Removes the historic v2.1-C archive from the Alpha 1 distribution; records source/hash origin without bundling that unresolved historical archive.
- Adds a versioned Memory Contract, license split/file map, full MIT and CC BY 4.0 texts, transition notice, code of conduct, source provenance register and controlled test checklist.

Alpha 0 is not modified by these changes. Alpha 1 remains non-canonical pending the user's controlled unpack/test sequence.
