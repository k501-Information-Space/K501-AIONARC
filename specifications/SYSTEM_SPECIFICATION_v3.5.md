# System Specification — K501 Integrated Pipeline & Proof Runtime v3.5.0

**Specification ID:** `K501-INTEGRATED-SYSTEM-3.5.0`  
**Status:** candidate specification for owner evaluation  
**Aggregate release intent:** Stable  
**Version boundary:** this aggregate version does not rewrite either component version or its normative contract.

## 1. Purpose and scope

The artifact combines two separately executable components behind one source tree and one top-level build/test workflow:

1. **AIONARC Pipeline v3 Alpha 1** observes a declared source tree, writes a separate append-only store, creates and verifies frames, captures supported text, maintains a derived index and QH256 application state, and offers lexical query and trace/history/diff operations.
2. **Proof/Evidence Runtime Alpha 1** records explicitly invoked child commands, run metadata, transcript, observed exit, and hashes. It does not interpret command meaning or close a project gate.
3. **Top-level integration** builds both components and tests the recorder by launching the exact AIONARC executable as a distinct child process. The integration layer does not link the binaries or make the source tree writable to AIONARC.

The aggregate does not include a model, semantic retrieval, embeddings, OCR, a remote service, signed attestations, a trusted timestamp authority, or a general-purpose content truth evaluator. Optional PDF extraction remains a subprocess dependency. Broader Runtime Architecture features not implemented in the embedded packages are excluded from the release claims.

## 2. Normative precedence

Within each component, its preserved source package contract and implementation remain authoritative for that component. The aggregate contract defines only composition and orchestration. If this specification conflicts with a component's normative manual, memory contract, mapping profile, or protocol status, the component's original contract governs until an explicit compatible revision is reviewed. No component file is silently amended by aggregate documentation.

## 3. Data and state boundaries

- AIONARC input source is observed read-only by design; its separate store is writable output. Source and store must not overlap.
- AIONARC frames preserve temporal structural records; extracted text and indexes are derived and remain source-bound.
- QH256 v2 algebra and 32-byte serialization are unchanged. The V3A1 mapping is application-specific; unused cells remain reserved/UNKNOWN as documented by the component.
- Proof/Evidence run records describe one declared process-exit check. The recorder's `gate_effect` is `NO_GATE_EFFECT_UNTIL_REVIEW`.
- Evidence status is **not** encoded into the AIONARC QH256 state. There is no aggregate status-to-cell mapping.
- A matching child exit code, checksum, or schema-valid record does not establish external truth, legal rights, gate acceptance, or release acceptance.

## 4. Aggregate acceptance contract

The local aggregate build passes its declared build profile only when all of these are true for the same source tree:

1. Both embedded package manifests validate before build.
2. Both C programs compile with their preserved package flags.
3. AIONARC's QH256, SHA-256, mapping and full regression tests pass.
4. The Proof/Evidence suite passes with the exact freshly built AIONARC executable and reports zero failed assertions.
5. Both packaged executables and their fresh builds are recorded by SHA-256.
6. JSON files, aggregate contracts, component and aggregate manifests, Index, and checksum sidecar validate.
7. The evidence run is retained with command, host, output, exit result and manifest.

This local technical acceptance profile is finite. Host acceptance, file-level rights/redistribution and same-host clean-copy reproducibility are assessed in the current validation report; cross-host acceptance and public release acceptance are not implied.

## 5. Aggregate identity and claims

The aggregate `3.5.0` identity binds the two component archive hashes recorded in `metadata/ARTIFACT_IDENTITY.json`, the exact preserved component trees, the top-level scripts/specifications, and build output hashes in the validation report. Any change to those inputs requires regenerated integrity files and fresh affected tests. The owner declared this integrated local artifact canonical at Unix `1793046600`. The declaration ratifies MIT for K501-authored software/code and CC BY 4.0 for K501-authored documentation/descriptive material as applied by `LICENSES/AGGREGATE_FILE_LICENSE_MAP.json`. This local designation does not publish a component repository or create a GitHub/Zenodo release. Third-party notices remain applicable.
