# Proof Before State Contract — Alpha 1

**Status:** Normative for `3.0.0-alpha.1`; non-canonical pending controlled owner test series.  
**Related:** `MEMORY_CONTRACT_V1.0.md`, `QH256_MAPPING_PROFILE.md`.

## Rule

A candidate state may become final `STATE.tsv` only after structural serialization, candidate reparsing, object/record recomputation, source-bound text roundtrip, deterministic transition reconstruction, Gateway validation, Base64 path roundtrip, text-status-aware index validation, QH256 recomputation/roundtrip, proof write/flush, and manifest completion have passed.

## Required order

1. Resolve and validate store, source/store disjointness, identifiers, absent target frame, and parent rules.
2. Observe the complete source tree read-only; sort bytewise, reject duplicate relative paths, hash file bytes twice, compare observed metadata, and record symlinks without following them.
3. Capture text only for declared candidates and record one of the normative statuses in the Memory Contract. The absence/failure of optional PDF extraction does not abort structural operation.
4. Write `STATE.candidate`, `TEXT.tsv`, `TRANSITION.tsv`, and `GATEWAY.tsv` into a unique staging directory. Validate transition and Gateway.
5. Re-read `STATE.candidate`; verify the state digest, object count, order, object ID, record hash and equality to the observed object set.
6. Re-read `TEXT.tsv`; validate text digest and binding to path/object/source digest. Empty text is an explicit zero-byte row and must parse without dropping its empty Base64 field.
7. Validate Base64 path roundtrip for every parsed candidate path.
8. Only then write final `INDEX.tsv`; validate every field/status/path/text binding and per-object QH256 value. Status must be restored before mapping in the verify path. The proof records the path Base64 roundtrip and validated index-status precondition explicitly.
9. Compute the aggregate QH256 from validated objects/index and the reconstructed transition; serialize and re-read the QH256 value.
10. Hash all artifacts available before the proof; write and flush `PROOF.tsv`. After proof is durable, rename `STATE.candidate` to `STATE.tsv`, write and flush `MANIFEST.sha256` over the completed eight-artifact set, then atomically promote staging to the absent frame ID and fsync the relevant directories where supported. The manifest cannot be complete before the final `STATE.tsv` name exists.

## Failure semantics

Any failed check prevents the frame from being promoted. Preserve incomplete staging evidence. No automatic correction, deletion or promotion is permitted. A nonzero exit is not a PASS. An exit-zero audit is not a gate PASS unless its stated assertions cover the gate.

## Verify semantics

Verification must:

- verify parent ancestry and chain digest;
- validate exact artifact hashes and schema/version IDs;
- parse `STATE.tsv` and recompute structural identities;
- load and bind `TEXT.tsv` entries;
- parse the `INDEX.tsv` row statuses, validate legal status/object/text combinations, restore each status before recomputing the object QH256 value;
- validate deterministic path order and Base64 roundtrip;
- reconstruct transition and Gateway;
- recompute the QH256 aggregate and compare it byte-for-byte/semantic-cell-for-semantic-cell with the stored value;
- verify `MANIFEST.sha256`.

`VERIFY PASS` establishes only these declared local invariants for that frame and ancestry; it does not assert factual truth, completeness of external knowledge, or protection against an adversary who can rewrite and rehash the whole store.
