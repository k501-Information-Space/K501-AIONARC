# Gateway — K501 AIONARC Integrated Pipeline & Proof Runtime

**Repository role:** `INTEGRATED_PIPELINE_AND_PROOF_RUNTIME`  
**Release:** `3.5.0` — published stable release  
**Gateway status:** `CURRENT_REPOSITORY_NAVIGATION / NAVIGATION_ONLY`  
**Operator time anchor:** `1793389236` — 2026-10-30T19:40:36Z / 2026-10-30T20:40:36+01:00

This Gateway is the root navigation layer for the repository. It does not replace the README, component documentation, release archive, manifests, source records or validation evidence. A navigation route is not a validation result or an authority transition.

## Entry sequence

1. [README.md](README.md) — human entry, published package identity and operating instructions.
2. [Gateway.json](Gateway.json) — machine-readable repository routes and boundaries.
3. [Index.md](Index.md) / [Index.json](Index.json) — derived aggregate path and byte inventory.
4. [MANIFEST.json](MANIFEST.json) — aggregate source-file manifest.
5. [SHA256SUMS.txt](SHA256SUMS.txt) and [SHA256SUMS.txt.sha256](SHA256SUMS.txt.sha256) — integrity records. Verify them against the exact repository checkout.

## Published aggregate release

- GitHub release: [v3.5.0](https://github.com/k501-Information-Space/K501-AIONARC/releases/tag/v3.5.0)
- Zenodo record: [23701613](https://zenodo.org/records/23701613), DOI [10.5281/zenodo.23701613](https://doi.org/10.5281/zenodo.23701613)
- Tagged source commit: `946836c3d160ce25d38d5456baa8ed586a1ecd05`
- Release archive SHA-256: `774cbf18d4674b657bafc859cda4b8b2a2dddcb2a283f70925510d4ae2d85223`

The aggregate release identity does not rewrite embedded component identities: AIONARC Pipeline remains `3.0.0-alpha.1`; Proof/Evidence Runtime remains `0.1.0-alpha.1`. The published tag and ZIP remain immutable. This Gateway and its README navigation link are later repository-head additions; they do not alter the tagged release archive.

## Repository components

- `components/K501-AIONARC-Pipeline/` — pipeline source, frames, contracts, validation and component-level records.
- `components/K501-Proof-Evidence-Runtime/` — process invocation, transcript/run metadata and evidence-file recording.
- Root build, integration and verification are coordinated by `Makefile` and `scripts/`.

Component documentation remains the primary source for component-specific behavior. The integrated repository Gateway does not collapse their identities or map evidence results into QH256 state.

## Time-scoped status records

`STATUS.md`, `metadata/GATE_STATUS.json` and `validation/VALIDATION_REPORT.md` retain their original source-time declarations. They were produced before the later publication state recorded by the current README and the v3.5.0 GitHub/Zenodo release. They are preserved historical records, not silently rewritten to match a later observation. Read each record with its own timestamp, source scope and claims.

## Integrity and rights boundary

The root Index is derived navigation and byte metadata; it is not semantic proof. The aggregate manifest and checksum list cover the scopes declared by their generators. A matching digest proves byte identity for the hashed object, not factual truth, rights clearance or canonicality.

The owner declaration and per-file license map are under `metadata/` and `LICENSES/`. Their scope is limited to the paths and material they explicitly classify. Preserve embedded component notices and third-party/QH256 terms; no aggregate navigation record overrides them.

## Related repositories

- [QH256](https://github.com/k501-Information-Space/QH256) — separate four-state model and Information Space structure reference.
- [AIONARC_CHANNEL](https://github.com/k501-Information-Space/AIONARC_CHANNEL) — append-only public signal layer; not this repository's source tree.
- [eArc](https://github.com/k501-Information-Space/eArc) — neutral development archive; repository and canonical authority are `NONE`.

## Build and verification entrypoints

Requirements and platform boundaries are declared in [README.md](README.md). The documented verification commands are `make test` and `make verify`; a Gateway read does not imply that either command was executed.
