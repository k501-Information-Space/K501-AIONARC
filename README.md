# K501 AIONARC Integrated Pipeline & Proof Runtime

**Version:** `3.5.0`  
**Release status:** Published stable release  
**GitHub release:** https://github.com/k501-Information-Space/K501-AIONARC/releases/tag/v3.5.0  
**Zenodo record:** https://zenodo.org/records/23701613  
**DOI:** https://doi.org/10.5281/zenodo.23701613  
**Tagged source commit:** `946836c3d160ce25d38d5456baa8ed586a1ecd05`  
**Release archive:** `K501-AIONARC-Integrated-Pipeline-and-Proof-Runtime_v3.5.0_Stable.zip`  
**Release archive SHA-256:** `774cbf18d4674b657bafc859cda4b8b2a2dddcb2a283f70925510d4ae2d85223`

The release archive was downloaded and checked locally: its size was 618,739 bytes, its SHA-256 matched the digest reported by the public GitHub release, and the ZIP integrity test passed.

**Release immutability:** This README status correction is intended as a separate follow-up commit on `main`. It does not change the `v3.5.0` tag, its tagged source commit, or the already published release ZIP. The README contained inside that archive remains as released.

This release integrates the K501 AIONARC Pipeline and K501 Proof/Evidence Runtime as one versioned, buildable source artifact. The two component trees are retained under `components/`; the top-level build coordinates them and runs the recorder integration test against the exact pipeline executable built in that tree.

The owner ratified MIT for K501-authored software/code and CC BY 4.0 for K501-authored documentation/descriptive material at Unix time `1793046600`. The per-file map and owner declaration are in `LICENSES/`. Embedded payloads and historical component status records remain unchanged; the aggregate ratification applies to this integrated package.

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

The aggregate v3.5.0 release is published on GitHub and deposited on Zenodo at the links above. The per-file license map and owner declaration remain in `LICENSES/`. Third-party and QH256 notices remain applicable. Future source or documentation changes should be recorded in separate commits and, when appropriate, a separately versioned release; they do not retroactively change the published v3.5.0 archive.

See [License Scope](LICENSE_SCOPE.md), [Owner Ratification](LICENSES/RIGHTSHOLDER_ATTESTATION.md), [Dynamic Gate Status](metadata/GATE_STATUS.json), and [Validation Report](validation/VALIDATION_REPORT.md).
