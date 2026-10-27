
# K501 Information Space Structure Specification v0.2

**Project:** `K501 AIONARC Pipeline`  
**Specification ID:** `K501-AIONARC-PIPELINE-IS-STRUCTURE-0.2`  
**Status:** `LOCAL_PACKAGE_PAYLOAD_IMPORTED_RECONCILIATION`  
**Action anchor:** `1792616559`

## Purpose

Specification v0.2 records the structural transition from the local bootstrap
to a tree containing the exact source-package payload. Specification v0.1
and Frame 000 remain preserved historical records.

## Control surface

The root control surface remains independent of the source package:

- `README.md`
- `Gateway.md` and `Gateway.json`
- `Index.md` and `Index.json`
- `frames/` — append-only records
- `specifications/` — current v0.2 and preserved v0.1
- `docs/` — operator instructions and project boundary
- `metadata/` — repository identity and source-archive provenance
- `references/`
- `runs/run.schema.json`
- `LICENSE_SCOPE.md`
- `MANIFEST.json`, `SHA256SUMS.txt`, and `SHA256SUMS.txt.sha256`

## Source package payload

- **Payload path:** `payload/K501_AIONARC_Pipeline_v3.0.0_AI_Memory_Runtime_C_alpha.1/`
- **Archive SHA-256:** `f14e203ad7bfa466d118780b06b69e59198e2fd917cb0e7e82222b83557df607`
- **Internal manifest:** PASS
- **Regular files imported:** 52
- **Payload tree SHA-256:** `716b9d02a1978149a592f717fa3b969e884fa3375e626adbdde3d86218130aa1`

The archive contents are preserved under `payload/` to avoid collisions with
the root control surface. The nested package manifest and licence/provenance
files remain part of the payload and are separately verifiable.

## Integrity contract

The root Index is a derived complete inventory of current file and directory
paths, not a content hash or semantic proof. The root `MANIFEST.json` records
SHA-256 and byte size for all local non-`.git` files except the root
`MANIFEST.json`, `SHA256SUMS.txt` and its sidecar. The root checksum list
covers the manifest and declared files; its sidecar covers the checksum list.

The nested source-package `SHA256SUMS.txt` is preserved and checked in its
own payload directory. `work/` and `repos/` must remain byte-identical outside
the Git metadata directory.

## Boundaries

- Frame 000 and specification v0.1 are not rewritten.
- Package source files are not edited by this import gate.
- Root-level `LICENSE` remains absent because a single blanket grant is not
  inferred from package-internal licence statements.
- Licence and redistribution clearance remain OPEN.
- Import does not execute a build or runtime, close integration gates,
  create a commit, add a remote, push, or publish a release.

**Invariants:** `APPEND_ONLY_FRAMES`, `NO_DRIFT`,
`SOURCE_BYTES_PRESERVED`, `DERIVED_IS_NOT_CANONICAL`,
`LOCAL_IMPORT_IS_NOT_RELEASE_ACCEPTANCE`.
