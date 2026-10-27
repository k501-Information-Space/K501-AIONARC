
# K501 Information Space Structure Specification v0.2

**Project:** `K501 Proof & Evidence Runtime`  
**Specification ID:** `K501-PROOF-EVIDENCE-RUNTIME-IS-STRUCTURE-0.2`  
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

- **Payload path:** `payload/K501_Proof_Evidence_Runtime_Alpha1_C/`
- **Archive SHA-256:** `c48fd8863b3b5b8889f46af1efbc768281490b60ab4433700b1331793dfcf435`
- **Internal manifest:** PASS
- **Regular files imported:** 21
- **Payload tree SHA-256:** `f3d2470ddbb73bee5cf1b0de8b3595f218c04f4a854bf1d4d1b43f9071a14dbd`

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
