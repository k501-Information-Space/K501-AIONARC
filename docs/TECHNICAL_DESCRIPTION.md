# Technical Description

## Software composition

The source components are native C11 programs. They remain in separate package roots and are compiled with their own Makefiles:

- `k501-memory`: AIONARC's single-writer local structural-memory CLI. It has commands for store initialization, scan/frame construction, verification, query, trace, history, diff and source verification. The exact command grammar and file-format contract remain in the embedded AIONARC manuals.
- `k501-proof-evidence`: controlled-run recorder. It validates its command form, creates a unique run directory, records metadata and executable identity where available, invokes the child with `execvp`, captures merged output, records exit and emits an evidence record plus checksums.
- `qh256.c` and related SHA-256 code are preserved within each component. There is no shared dynamically loaded library or ABI between the executables.

## Build profile

AIONARC fixes `-march=x86-64 -mtune=generic`, compiles with `-std=c11`, warning flags and `-Werror`, and retains its disclosed `-Wno-misleading-indentation` exception. Its shipped binary requires Linux x86-64 and GLIBC symbols through 2.34. The recorder uses C11 and stricter conversion/shadow warning flags with `-Werror`. Compiler flags and ABI are not merged or silently overridden by the aggregate Makefile.

The top-level Makefile invokes component builds in order, builds both packages, copies the exact outputs into `dist/linux-x86_64/`, then tests AIONARC and the recorder-to-runtime child process. The recorder suite expects the built executable supplied as `RUNTIME_BIN`.

## Artifact and evidence formats

Each source package retains its original internal `SHA256SUMS.txt`, schemas, specifications and version. The aggregate adds `ARTIFACT_IDENTITY.json`, a typed integration contract, a machine-readable system specification, a QH256 mapping-boundary record, an Index, a scoped manifest and a root checksum file with sidecar. These integrity records describe bytes and structure; they do not attest authorship or correctness.

The Proof/Evidence Runtime's Evidence Record schema is preserved at the component path and copied to `schemas/evidence-record.schema.json`. Evidence Protocol v0.1.0 is still proposed. The aggregate does not promote it to a ratified standard or alter its `NO_GATE_EFFECT_UNTIL_REVIEW` boundary.

## Validation claims

The package tests check finite regression cases described in `components/K501-AIONARC-Pipeline/.../docs/VALIDATION_REPORT.md` and `components/K501-Proof-Evidence-Runtime/.../BUILD_VALIDATION_REPORT.md`. The aggregate integration case proves that the recorder can launch the tested AIONARC executable as a child process and record the declared outcome. It is not a proof of the complete QH256 state space, external truth, universal portability, hostile-store resistance, target-host compatibility, or legal redistribution rights.
