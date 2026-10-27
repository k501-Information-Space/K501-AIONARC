
# K501 Proof & Evidence Runtime — K501 Information Space

**Project ID:** `K501-PROOF-EVIDENCE-RUNTIME`  
**Candidate version:** `0.1.0-alpha.1`  
**State:** `LOCAL_PACKAGE_PAYLOAD_IMPORTED_LICENSE_AND_ACCEPTANCE_OPEN`  
**Intended GitHub target name:** `k501-Information-Space/K501-Proof-Evidence-Runtime`

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

- Payload: `payload/K501_Proof_Evidence_Runtime_Alpha1_C/`
- Source archive SHA-256: `c48fd8863b3b5b8889f46af1efbc768281490b60ab4433700b1331793dfcf435`
- Internal package manifest: PASS
- Verified internal manifest entries: 20
- Imported regular files: 21
- Payload tree SHA-256: `f3d2470ddbb73bee5cf1b0de8b3595f218c04f4a854bf1d4d1b43f9071a14dbd`

Start with the package's preserved [README](../payload/K501_Proof_Evidence_Runtime_Alpha1_C/README.md) when reviewing
the underlying package itself. The payload is kept separate from the root
control surface to prevent filename collisions.

## Status boundary

Separate Proof & Evidence Runtime Alpha-1 candidate. The package is staged locally with source bytes preserved from the verified ZIP. It does not replace tools/k501-run.sh or establish combined integration acceptance with the AIONARC pipeline.

**Integration status:** PARTIAL/OPEN: historical standalone tests passed within their recorded scope. Combined integration and the remaining G17-F conformance gaps are not closed by this import.

The ZIP payload was copied locally with its source-file bytes preserved.
This import does not establish runtime acceptance, integrated operation,
a package-wide licence, redistribution clearance, public canonicality, or
release readiness. No root-level `LICENSE` is inferred from the nested
package's licence files.

## Local workflow

`work/K501-Proof-Evidence-Runtime` is the working tree. `repos/K501-Proof-Evidence-Runtime` is
the separate local Git root. Tracked-content files must match byte-for-byte.
No remote is configured; no commit, push, or publication was performed.

## Principles

`PROOF_BEFORE_STATE` · `RECONSTRUCT_BEFORE_EXTEND` · `APPEND_ONLY` ·
`NO_DRIFT` · `NO_PHANTOM` · `NO_INTERPRETATION` ·
`DERIVED_IS_NOT_CANONICAL`
