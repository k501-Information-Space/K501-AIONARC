
# K501 Information Space Structure Specification v0.1

**Project:** `K501 AIONARC Pipeline`  
**Specification ID:** `K501-AIONARC-PIPELINE-IS-STRUCTURE-0.1`  
**Status:** `LOCAL_BOOTSTRAP_CANDIDATE`  
**Action anchor:** `1792615629`

## Control surface

- `README.md`: human landing page and current boundary.
- `Gateway.md` and `Gateway.json`: human/machine entrypoints.
- `Index.md` and `Index.json`: derived inventory of the current local tree.
- `frames/`: append-only human/machine structural declarations.
- `specifications/`: versioned structural contracts.
- `docs/`: operating instructions and project boundary.
- `metadata/`: identity, source bindings, and release-manifest template.
- `references/`: known source paths and previously reported archive hashes.
- `runs/run.schema.json`: shared K501 run record contract.
- `MANIFEST.json`, `SHA256SUMS.txt`, and its sidecar: scoped integrity records.
- `LICENSE_SCOPE.md`: license status; it does not grant a license.

## Project payload

The Alpha-1 source package has not been imported by this gate. Payload files,
license notices, third-party records, source manifests, build scripts, and
tests must be handled by the package-copy and validation gates. The working
source tree and its local Git mirror are separate from the package source.

## Integrity and canonicality

The Index is a derived path inventory, not a content hash or semantic proof.
The manifest's declared scope is limited to the present bootstrap control
surface and identity records. A local unborn branch does not establish
public or canonical repository status.

The source package and its current candidate identity must be rechecked
before copy. After any controlled tree change, regenerate Index, manifest,
checksums, and the sidecar. Preserve old frames and append new transitions.
