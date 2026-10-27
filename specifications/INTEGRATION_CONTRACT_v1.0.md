# Integration Contract v1.0

**Contract ID:** `K501-PIPELINE-EVIDENCE-INTEGRATION-1.0`  
**Status:** proposed aggregate contract, subject to owner evaluation

## Components and interface

| Provider | Consumer | Interface | Side effect |
|---|---|---|---|
| AIONARC executable | Proof/Evidence recorder test harness | Separate child process, argv `k501-memory qh256-test` | The child self-test runs; the test harness writes only to its isolated temporary test workspace. |
| Top-level Makefile | Both package Makefiles | `make clean all`, declared package targets and `RUNTIME_BIN=<absolute-built-path>` | Build outputs in ignored component `build/` directories and copied executables in `dist/linux-x86_64/`. |
| Aggregate validator | Frozen package files | Read manifests, JSON/spec records and output hashes | No component source or historical source run is edited by verification. |

## Required order

1. Validate the exact source/component manifests.
2. Build the AIONARC executable using its own fixed x86-64 baseline.
3. Build the Proof/Evidence recorder using its own strict C11 flags.
4. Run AIONARC self-tests and regression suite.
5. Run the recorder test suite with the freshly built AIONARC path supplied as `RUNTIME_BIN`.
6. Verify package manifests, executable hashes, aggregate schemas, Index, manifest, and checksum sidecar.

## Interface and failure rules

- The recorder MUST invoke the supplied executable as a child process and MUST NOT link it into the recorder.
- The caller MUST use the absolute path of the AIONARC executable built from this same aggregate source tree for the integration test.
- A child process exit match proves only the declared process criterion. The recorder's evidence record keeps `NO_GATE_EFFECT_UNTIL_REVIEW`; a human gate evaluation remains separate.
- A recorder failure, transcript capture failure, missing binary, hash mismatch, or any nonzero required test stops aggregate verification. Preserve its run evidence and create a new run after correction.
- Neither component may be silently substituted by another version, executable, archive, or external store.
- The Evidence Protocol v0.1.0 remains proposed/not ratified. Packaging it does not ratify it.

## Non-goals

This contract does not define a unified persistent store, cross-component state schema, evidence-status mapping into QH256, API/ABI between modules, network protocol, concurrent writer behavior, or semantic interpretation. Those would require separate versioned specifications and tests.
