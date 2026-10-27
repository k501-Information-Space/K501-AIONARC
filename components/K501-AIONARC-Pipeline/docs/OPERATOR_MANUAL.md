
# Operator Manual — K501 AIONARC Pipeline

## Current state

The verified ZIP payload is staged under `payload/K501_AIONARC_Pipeline_v3.0.0_AI_Memory_Runtime_C_alpha.1/`. The archive's
internal `SHA256SUMS.txt` has passed. The root Index and manifest describe the
resulting local tree, including the imported payload.

## Read sequence

1. Read `README.md`.
2. Read `Gateway.md` and `Gateway.json`.
3. Read `Index.md` and `Index.json`.
4. Read Frame 001 and its JSON record; preserve Frame 000 unchanged.
5. Read the current structure specification v0.2.
6. Verify root integrity from this repository root:
   `sha256sum -c SHA256SUMS.txt`
7. Verify the imported package separately:
   `cd payload/K501_AIONARC_Pipeline_v3.0.0_AI_Memory_Runtime_C_alpha.1 && sha256sum -c SHA256SUMS.txt`
8. Review `LICENSE_SCOPE.md`, `metadata/source-archive-records.json`, and
   the package-specific licence/provenance files before any redistribution.

## Mutation boundary

This import gate does not edit payload file contents. Recompute the derived
root Index, manifest and checksum set after any approved tree transition.
Append structural frames instead of rewriting historical records.

No build or runtime execution, Git commit, remote configuration, push, GitHub
repository creation, or Zenodo publication is authorized by this gate.
