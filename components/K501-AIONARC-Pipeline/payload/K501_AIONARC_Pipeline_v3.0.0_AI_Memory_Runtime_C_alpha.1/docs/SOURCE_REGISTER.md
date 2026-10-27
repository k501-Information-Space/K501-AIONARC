# Source Register — Alpha 1

1. K501 canonical consolidated system specification, 2026-09-27, supplied from the K501 document library. Used for core axioms, QH256 definition, frame boundaries and identity/hash distinctions.
2. K501 Pipeline v3 Gate Roadmap v1.0, time anchor `1792052674`, 2026-10-15. Used for workspace and gate boundaries.
3. K501 Information Space Runtime Architecture Draft v0.1, 2026-10-12. Used for module boundaries, runtime scope, model-admission and non-canonical-data principles; it remains a design source and is not treated as proof of implemented features.
4. K501 State Memory `1792089824`, 2026-10-15. Used as a historical G4 state report, not as a replacement for newer user-provided test outputs.
5. Alpha 0 source package `K501_AIONARC_Pipeline_v3.0.0_AI_Memory_Runtime_C` and files/hashes cited by the run outputs. Alpha 0 remains the immutable historical baseline. Alpha 1 was prepared in a separate copy.
6. Public K501 QH256 repository root README and MIT license: `https://github.com/k501-Information-Space/QH256`, current root `LICENSE` and `README.md` reviewed through the GitHub repository connector. Exact current online paths for individual imported core sources did not resolve during this pass; the package makes no independent claim of an exact current-branch source match.
7. NIST FIPS 180-4, Secure Hash Standard, `https://csrc.nist.gov/pubs/fips/180-4/upd1/final`, referenced for the SHA-256 algorithm only; Alpha 1 `src/sha256.c` is a fresh formatted implementation in this package and is tested against standard known vectors.
8. CC BY 4.0 legal code, `https://creativecommons.org/licenses/by/4.0/legalcode`, and SPDX license identifier `CC-BY-4.0`, `https://spdx.org/licenses/CC-BY-4.0.html`; these sources describe the standard license text, not third-party code included in the runtime.

This is a package preparation register, not an independent third-party audit. Facts, declarations, derived test outcomes and unresolved items are separated in `docs/STATUS_REGISTER.md` and `LICENSES/FILE_LICENSE_MAP.md`.
