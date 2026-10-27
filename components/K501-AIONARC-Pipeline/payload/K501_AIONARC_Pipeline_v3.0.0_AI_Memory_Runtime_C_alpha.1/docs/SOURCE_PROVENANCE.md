# Source and Provenance Register — Alpha 1

**Status:** package-local provenance statement for a non-canonical candidate; it is not an independent legal opinion or a cryptographic signature by the rights holder.

## 1. Alpha 0 input and preservation

Alpha 1 was prepared from an extracted copy of `K501_AIONARC_Pipeline_v3.0.0_AI_Memory_Runtime_C` Alpha 0. Its original archive is `K501_AIONARC_Pipeline_v3.0.0_AI_Memory_Runtime_C.zip` (historical recorded SHA-256 `f64a796989afb41da19b5aafa4a5d3b2829198a1f6bc2283abad5710644f0230` in prior source documentation). The Alpha 0 package and its separate runs are not modified by this package operation.

Previous Alpha 0 preparation documented byte-identical copies of `include/qh256.h`, `src/qh256.c`, `src/sha256.h`, and `src/sha256.c` relative to the archived v2.1-C package. Alpha 1 retains the QH256 v2 core implementation and API, but rewrites its SHA-256 implementation in `src/sha256.c` as a clean, independently formatted implementation against the algorithm specified in NIST FIPS 180-4. That rewrite is tested against the standard empty-string and `abc` known vectors, plus incremental update.

The original v2.1-C archive is deliberately **not** redistributed in Alpha 1. This package retains the historical archive hash as an origin reference only. The exact historical archive itself is not an Alpha 1 runtime dependency.

## 2. QH256 source and public repository

The public repository `https://github.com/k501-Information-Space/QH256` presents a root `LICENSE` containing the MIT License and a README defining QH256 as a deterministic four-state 256-bit state representation. The visible online root license hash returned during this preparation was `63cb704e1c474dc8ecefbfbf2c7afd562a5bdd9a`; the README blob hash was `7d77900d9a057e4c95fef17be10b720a123197ae`. File-level source paths queried during this pass did not resolve at the current default-branch paths, so these facts are recorded as repository context, not asserted as an independent byte-for-byte online source match.

The Alpha 1 package uses its explicit file-level license map and relies on the project maintainer's assertion that the code licensed by the accompanying MIT license is material the maintainer owns or is authorized to license. If any component is later shown to be third-party code with separate terms, that component must be excluded or its exact license/attribution added before public redistribution.

## 3. Dependencies

- Required runtime dependency: libc / compatible Linux ELF loader.
- Optional external PDF utility: `pdftotext` (Poppler tools) if installed; not bundled, and not required for the structural core.
- Build-time tools: C11-capable compiler and Make. The shipped binary's qualified contract is separately recorded; a reproducible build is not claimed.
- No runtime use of OpenSSL, Python packages, SQL, CUDA, network services or llama.cpp.

## 4. Provenance vocabulary

- `OBSERVED`: directly present in the package or command output.
- `RECONSTRUCTED`: re-created from recorded source and hash identifiers.
- `DECLARED`: rights-holder or maintainer statement that has not been independently adjudicated.
- `UNRESOLVED`: evidence insufficient for a strong conclusion.

This register does not replace package checksums, per-file notices, the owner declaration or the later target-host test report.
