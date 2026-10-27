# K501 AIONARC Integrated Pipeline & Proof Runtime

**Selected artifact name:** K501 AIONARC Integrated Pipeline & Proof Runtime  
**Version:** `3.5.0`  
**Release intent:** Stable  
**State:** canonical local aggregate by owner declaration; GitHub publication not performed  
**User time anchor:** Unix `1793046600` — 2026-10-26 20:30:00 UTC / 21:30:00 CET

This candidate assembles the existing K501 AIONARC Pipeline and K501 Proof/Evidence Runtime as one versioned, buildable source artifact. The two upstream project trees are retained intact under `components/`; the top-level build coordinates them and runs the recorder integration test against the exact pipeline executable built in that tree.

The owner ratified MIT for K501-authored software/code and CC BY 4.0 for K501-authored documentation/descriptive material at Unix time `1793046600`. The per-file map and owner declaration are in `LICENSES/`. Embedded payloads and historical component status records remain unchanged; the aggregate ratification applies to this integrated package. The local artifact is owner-designated canonical in this workspace; it is not a published GitHub release.

The aggregate version `3.5.0` is an integration-package identity. It does not rewrite the embedded component identities: AIONARC remains `3.0.0-alpha.1`, and Proof/Evidence Runtime remains `0.1.0-alpha.1`. See `metadata/ARTIFACT_IDENTITY.json` and `specifications/SYSTEM_SPECIFICATION_v3.5.json` for the bound source archive digests and scope.

## Build and verify

Requirements: POSIX/Linux, GNU Make, a C11 compiler, Python 3, and `sha256sum`. AIONARC’s shipped executable is qualified for Linux x86-64 and GLIBC symbols through 2.34. `pdftotext` is optional and only enables the PDF adapter.

```sh
make clean
make all
make test
make verify
```

Built release executables are staged in `dist/linux-x86_64/`. `make test` runs the AIONARC core and full regression suite, then runs the Proof/Evidence Runtime test suite with the freshly built AIONARC executable supplied as a child process. `make verify` additionally checks the source component manifests, aggregate schemas/specification, index, root manifest and checksums.

See [Operator Manual](docs/OPERATOR_MANUAL.md), [Technical Description](docs/TECHNICAL_DESCRIPTION.md), [System Specification](specifications/SYSTEM_SPECIFICATION_v3.5.md), [Integration Contract](specifications/INTEGRATION_CONTRACT_v1.0.md), and [QH256 Mapping Profile](specifications/QH256_MAPPING_PROFILE_v3.5.md).

## Component boundaries

- **AIONARC Pipeline:** read-only structural source observation, append-only frames, source-bound text extraction, verification, exact lexical query, trace/history/diff, and its versioned QH256 V3A1 application mapping.
- **Proof/Evidence Runtime:** controlled process invocation, transcript and host/run metadata capture, result recording, and evidence-file digests. Its recorded process result does not decide a project gate.
- **Integration:** the recorder launches the pipeline as a separate process for `qh256-test`; it does not link or replace it. Evidence statuses are not mapped into QH256.

The detailed boundaries and limitations are preserved in each component’s original documentation under `components/`. The Evidence Protocol v0.1.0 remains a proposed reference specification, not a ratified protocol.

## Release and rights state

The owner has ratified the aggregate file-license map and authorized distribution of mapped K501-owned material under MIT/CC BY 4.0. Third-party and QH256 notices remain applicable. The local canonical designation is recorded in `metadata/ARTIFACT_IDENTITY.json`; it does not imply GitHub or Zenodo publication. The target component repositories were not found through the configured GitHub connection, and no aggregate destination is defined. GitHub publication and Zenodo publication gates therefore remain open.

See [License Scope](LICENSE_SCOPE.md), [Owner Ratification](LICENSES/RIGHTSHOLDER_ATTESTATION.md), [Dynamic Gate Status](metadata/GATE_STATUS.json), and [Validation Report](validation/VALIDATION_REPORT.md).
