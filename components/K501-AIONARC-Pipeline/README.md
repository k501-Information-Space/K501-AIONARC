
# K501 AIONARC Pipeline — K501 Information Space

**Project ID:** `K501-AIONARC-PIPELINE`  
**Candidate version:** `3.0.0-alpha.1`  
**State:** `LOCAL_PACKAGE_PAYLOAD_IMPORTED_LICENSE_AND_ACCEPTANCE_OPEN`  
**Intended GitHub target name:** `k501-Information-Space/K501-AIONARC-Pipeline`

## Current entrypoints

- Gateway: [Gateway.md](Gateway.md) / [Gateway.json](Gateway.json)
- Complete local path Index: [Index.md](Index.md) / [Index.json](Index.json)
- Current specification: [v0.2 Markdown](specifications/K501_INFORMATION_SPACE_STRUCTURE_v0.2.md) / [machine record](specifications/K501_INFORMATION_SPACE_STRUCTURE_v0.2.json)
- Current frame: [Frame 001](frames/FRAME_001_PACKAGE_IMPORT.md) / [machine record](frames/FRAME_001_PACKAGE_IMPORT.json)
- Bootstrap history: [Frame 000](frames/FRAME_000_BOOTSTRAP.md) and [specification v0.1](specifications/K501_INFORMATION_SPACE_STRUCTURE_v0.1.md)
- Operator manual: [docs/OPERATOR_MANUAL.md](docs/OPERATOR_MANUAL.md)
- Project boundary: [docs/PROJECT_BOUNDARY.md](docs/PROJECT_BOUNDARY.md)
- Source archive records: [metadata/source-archive-records.json](metadata/source-archive-records.json)
- Licence scope: [LICENSE_SCOPE.md](LICENSE_SCOPE.md)
- Root manifest: [MANIFEST.json](MANIFEST.json)
- Root checksums: [SHA256SUMS.txt](SHA256SUMS.txt) / [sidecar](SHA256SUMS.txt.sha256)

## Imported source package

- Payload: `payload/K501_AIONARC_Pipeline_v3.0.0_AI_Memory_Runtime_C_alpha.1/`
- Source archive SHA-256: `f14e203ad7bfa466d118780b06b69e59198e2fd917cb0e7e82222b83557df607`
- Internal package manifest: PASS
- Verified internal manifest entries: 51
- Imported regular files: 52
- Payload tree SHA-256: `716b9d02a1978149a592f717fa3b969e884fa3375e626adbdde3d86218130aa1`

Start with the package's preserved [README](../payload/K501_AIONARC_Pipeline_v3.0.0_AI_Memory_Runtime_C_alpha.1/README.md) when reviewing
the underlying package itself. The payload is kept separate from the root
control surface to prevent filename collisions.

## Status boundary

Separate AIONARC Pipeline Alpha-1 candidate. The package is staged locally with source bytes preserved from the verified ZIP. Candidate binding and downstream acceptance gates remain open.

**Integration status:** OPEN: current candidate binding, G4 formal closeout, G5-A mapping/contract conformance, G5-C rights/provenance clearance, bounded corpus validation, and reproducibility closeout remain unresolved.

The ZIP payload was copied locally with its source-file bytes preserved.
This import does not establish runtime acceptance, integrated operation,
a package-wide licence, redistribution clearance, public canonicality, or
release readiness. No root-level `LICENSE` is inferred from the nested
package's licence files.

## Local workflow

`work/K501-AIONARC-Pipeline` is the working tree. `repos/K501-AIONARC-Pipeline` is
the separate local Git root. Tracked-content files must match byte-for-byte.
No remote is configured; no commit, push, or publication was performed.

## Principles

`PROOF_BEFORE_STATE` · `RECONSTRUCT_BEFORE_EXTEND` · `APPEND_ONLY` ·
`NO_DRIFT` · `NO_PHANTOM` · `NO_INTERPRETATION` ·
`DERIVED_IS_NOT_CANONICAL`
