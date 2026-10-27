# K501 Information Space — Memory Contract v1.0

**Document ID:** `K501-IS-MEMORY-CONTRACT-1.0`  
**Runtime:** K501 AIONARC Pipeline v3 `3.0.0-alpha.1`  
**Mapping profile:** `QH256-K501-AIONARC-MEMORY-V3A1`  
**Time anchor:** Unix `1792121537` · 2026-10-16 03:32:17 UTC / 05:32:17 CEST  
**Status:** NORMATIVE FOR THIS ALPHA 1 CANDIDATE; NON-CANONICAL PENDING OPERATOR TESTS  
**Licensing:** this specification is CC BY 4.0 under the package file map.

## 1. Authority and scope

This document is the controlling engineering contract for the Alpha 1 runtime package. Source code, mapping profile, formats, test expectations, and operator guidance must agree with it. If they disagree, do not silently choose whichever implementation is convenient: record the conflict, keep the gate `OPEN`, and correct a new candidate while preserving prior evidence.

This document is a technical specification, not a legal agreement and not a declaration that the runtime is canonical, secure against an all-powerful local attacker, semantically intelligent, or production-ready. `EXIT_ZERO` means the controlled command completed; a gate becomes `PASS` only when its acceptance conditions are met by retained evidence.

## 2. Governing invariants

1. **PROOF_BEFORE_STATE:** a candidate frame is not promoted into its final frame path until structural serialization, text binding, transition reconstruction, index and Gateway validation, QH256 calculation, proof writing, and required checks have succeeded.
2. **RECONSTRUCT_BEFORE_EXTEND:** a predecessor must verify before a child frame uses it. Ambiguous tips are refused rather than guessed.
3. **APPEND_ONLY:** frame identifiers are unique within a store. Existing frames are never overwritten. A failed staging directory is evidence, not a committed frame, and must not be auto-promoted or auto-deleted.
4. **NO_DRIFT:** source bytes and relevant filesystem metadata are checked during observation; a detected change aborts the operation. `verify-source` checks a selected stored structural snapshot against the currently observed source tree.
5. **OBSERVE_BEFORE_MUTATE:** source data are read-only inputs. The runtime writes only into a separately located store and rejects overlapping source/store trees.
6. **NO_INTERPRETATION / STRUCTURE_PRECEDES_MEANING:** this runtime captures and indexes bytes; it does not assert the truth of prose or produce semantic claims.
7. **DERIVED_IS_NOT_CANONICAL:** `TEXT.tsv`, `INDEX.tsv`, `GATEWAY.tsv`, transitions, query hits, and the QH256 memory mapping are derived runtime artifacts. `STATE.tsv` records observed structural facts, not extracted prose or model output.
8. **TRACEABILITY_REQUIRED:** every retrieval result must point to a verified frame and source object, with identity and digests available for inspection.
9. **NO_PHANTOM:** no result, status, permission, extractor success, or canonical admission may be invented where evidence is missing.
10. **SILENCE_IS_A_VALID_SIGNAL:** no captured text or no query hit is not proof that the source contains no meaningful information outside this runtime's support boundary.
11. **DENSIFICATION_WITHOUT_ERASURE:** adding a new valid observation does not rewrite historical frames. Corrections require a new versioned frame/profile or new package, not silent mutation.

## 3. Runtime authority model

The operator declares a source directory, a store directory, a stable root ID, and a frame ID/parent. The runtime can only make claims about filesystem objects within that declared source scope and the declared format/profile rules.

- Source bytes and filesystem objects are the observed input.
- `STATE.tsv` is the structural snapshot for a frame.
- `TEXT.tsv` contains successfully captured derived text, bound to object identity, source digest, path and text digest.
- `INDEX.tsv` is the validated derived projection, including each object's text-capture status and per-object QH256 mapping.
- `QH256.hex` is an aggregate four-state summary and is not the complete memory or history.
- `TRANSITION.tsv` records deterministic structural change events relative to a verified parent.
- `GATEWAY.tsv` is orientation/lookup metadata; it is not a semantic authority source.
- `FRAME.tsv`, `PROOF.tsv`, and `MANIFEST.sha256` bind the frame metadata, validation claims, and artifact bytes.
- Query results are derived output and never become source or canonical state automatically.

No model, LLM, OCR, embeddings, or semantic inference is executed by this package. `pdftotext` is an optional external adapter for PDF text capture only. Its absence must not prevent structural scan, source hashing, frame creation, verification, trace/history/diff, or query of other captured text.

## 4. Object observation, ordering and identity

### 4.1 Object classes

Every observed node is classified by `lstat` as `file`, `directory`, `symlink`, or `other`. Symbolic links are recorded as links; their targets are hashed as link-target bytes and are not followed into another tree. Files, directories, links, and other special objects remain structural inventory records even when they are not text candidates.

### 4.2 Scope and path order

- Source and store must resolve to disjoint, non-overlapping directory trees.
- The source root itself uses relative path `.`.
- Relative paths are sorted bytewise using the implementation's C-string comparison; duplicate paths are refused.
- A Unix path cannot contain NUL. Path bytes are retained as observed; the runtime does not infer rename continuity or semantic path equivalence.
- Base64 is a transport representation for file fields, not encryption or data protection.

### 4.3 Identity domains are not interchangeable

The following values are separate and have separate defined inputs:

- `object_id`: domain-separated SHA-256 over the declared object-ID domain, NUL separators, stable `root_id`, relative path and object type. For this Alpha 1 implementation the object-ID domain and formula are unchanged from Alpha 0 so identical root/path/type tuples keep the same object identity.
- `record_hash`: SHA-256 over the declared object-record serialization, including root ID, Base64 relative path, object type, mode, size, mtime and content digest. The record schema/formula is unchanged in this profile migration.
- `content_sha256`: SHA-256 of regular-file bytes or symlink target bytes; directories and unsupported special nodes do not receive a made-up content digest.
- `text_sha256`: SHA-256 of captured text bytes, not of the source file and not of semantic meaning.
- `state_sha256`: SHA-256 of the exact serialized `STATE.tsv` bytes.
- `chain_sha256`: Alpha 1 domain-separated SHA-256 over the parent chain digest (zero bytes for the initial frame) and current state digest. Alpha 1 uses a V3A1 chain domain so the new frame-format namespace is explicit.
- `QH256`: a 32-byte, 256-bit state representation produced by QH256 v2 operations and the A1 application mapping. It is not a SHA-256 digest.
- Artifact SHA-256: byte digests of the exact stored frame artifacts.

No field may be used as a substitute for another merely because both are 64 hexadecimal characters.

## 5. Frame lifecycle

The state transition is:

`UNOBSERVED → OBSERVED → CANDIDATE_SERIALIZED → CANDIDATE_RECONSTRUCTED → DERIVED_ARTIFACTS_VALIDATED → QH256_RECOMPUTED → PROOF_DURABLE → STATE_PROMOTED → MANIFEST_DURABLE → FRAME_DIRECTORY_PROMOTED`.

A step only advances when its acceptance condition passes. A failure leaves the frame uncommitted. The runtime must not rename `STATE.candidate` to `STATE.tsv` before `PROOF.tsv` is written and flushed. The final staging directory is promoted by atomic directory rename to an absent frame-ID directory. Existing frame IDs are refused.

For a child frame, the parent frame and ancestry must verify; root ID must agree, child sequence must be exactly parent sequence + 1, and the parent state digest must match. `parent=none` is only permitted when the store has no prior frame. When there is more than one leaf/tip, `latest` is ambiguous and the runtime must request a specific parent/frame ID.

## 6. Structural and derived schemas

A frame contains the versioned Alpha 1 schemas for `FRAME.tsv`, `STATE.tsv`, `TEXT.tsv`, `INDEX.tsv`, `TRANSITION.tsv`, `GATEWAY.tsv`, `QH256.hex`, `PROOF.tsv`, and `MANIFEST.sha256`.

- `STATE.tsv` contains sorted structural object records. It does not contain captured prose or `text_status`.
- `TEXT.tsv` contains a row only when a text byte sequence was captured, including an empty byte sequence. Each row binds object ID, source content digest, text digest, extractor ID, Base64 path, byte length and Base64 text. Empty text has a zero byte length and an empty final Base64 field; the parser must preserve this field rather than discard it.
- `INDEX.tsv` contains one row for every observed object, its structural identities, optional text digest/extractor/searchability, Base64 path, `text_status`, and per-object QH256 value. It is derived but its exact bytes are bound by `PROOF.tsv` and `MANIFEST.sha256`.
- `TRANSITION.tsv` is reconstructed from parent and current state. It is not an inferred rename graph.
- `GATEWAY.tsv` provides object/frame navigation only.
- `PROOF.tsv` asserts only the enumerated local validation steps and must not claim external truth.

The index status is a validated observation input to QH256 reconstruction. On verification, the implementation must parse and validate each status from `INDEX.tsv`, bind it to the corresponding object and `TEXT.tsv` entry (if any), restore it into the in-memory object record, and only then recompute the per-object QH256 value and aggregate. A status must never be inferred from absence of text alone when the stored index explicitly carries a finer outcome.

## 7. Text-capture taxonomy (normative)

The status string is a closed vocabulary; new values require a new profile/version and tests. Alpha 1 statuses are:

- `NOT_CANDIDATE`: the object is not a supported text-capture candidate. It must have no `TEXT.tsv` row.
- `CAPTURED`: a candidate's non-empty text bytes were captured, UTF-8 validated, NUL-checked, bound to the current source digest, and written to `TEXT.tsv`.
- `CAPTURED_EMPTY`: a candidate's zero-length text result was captured successfully. It has a valid text digest and `TEXT.tsv` row; it is available evidence but not searchable.
- `SKIPPED_TOTAL_LIMIT`: the candidate was not processed because the frame text budget was already exhausted, or non-PDF candidate bytes did not fit the remaining budget. When a PDF extractor was actually run successfully but its result did not fit, use `PDF_CAPTURE_SKIPPED_TOTAL_LIMIT` instead.
- `SKIPPED_FILE_LIMIT`: a non-PDF text candidate exceeds the per-file read cap. Its structural record remains.
- `READ_FAILED`: a non-PDF candidate could not be read successfully.
- `INVALID_UTF8`: the produced candidate bytes failed the defined UTF-8 validator. Use only when that validator explicitly rejects the bytes.
- `EMBEDDED_NUL`: bytes passed UTF-8 validation but contain U+0000/NUL. This runtime's C-string-oriented text/index layer rejects such bytes from capture; this status is not synonymous with invalid UTF-8.
- `PDF_EXTRACTOR_UNAVAILABLE_OR_FAILED`: the optional PDF adapter could not be started or exited unsuccessfully, or extractor setup failed.
- `PDF_OUTPUT_FILE_LIMIT`: the PDF child completed successfully, but its output exceeded the per-file limit.
- `PDF_CAPTURE_SKIPPED_TOTAL_LIMIT`: the PDF child completed successfully, but the extracted result did not fit the remaining total frame text budget.
- `PDF_CAPTURE_READ_FAILED`: the PDF child completed successfully but the parent process could not finish reading the pipe result.

Precedence: invalid UTF-8 validation is checked before the embedded-NUL rejection. Thus if bytes contain both malformed UTF-8 and NUL, `INVALID_UTF8` is recorded because UTF-8 validation has failed. If bytes are UTF-8-valid with NUL, `EMBEDDED_NUL` is recorded. Both remain structural-only and have no text row.

### 7.1 Status validation constraints

1. A non-candidate must be `NOT_CANDIDATE` and have no text row.
2. A text row must be exactly `CAPTURED` when byte length is positive or `CAPTURED_EMPTY` when byte length is zero.
3. A candidate without a text row must not be marked `NOT_CANDIDATE`, `CAPTURED`, or `CAPTURED_EMPTY`.
4. `SKIPPED_FILE_LIMIT` and `READ_FAILED` are for non-PDF candidates only.
5. PDF-specific statuses are allowed only for PDF candidates.
6. A successful text row must validate text SHA-256, path, source content digest and object identity; source bytes must be rechecked after text extraction.
7. Unknown, contradictory, illegal, or extra status rows cause verification refusal.

## 8. QH256 v2 core and A1 mapping

The QH256 core remains unchanged: 128 cells, 2 evidence bits per cell, 256 bits/32 bytes; `UNKNOWN=00`, `FALSE=01`, `TRUE=10`, `GUARD=11`. The v3 application profile assigns cells 0–63; cells 64–127 stay `UNKNOWN`/reserved. A1 does not alter the QH256 operators, serialization, or 32-byte wire representation.

`QH256-K501-AIONARC-MEMORY-V3A1` is the mapping authority for cells 0–63. The complete table is in `docs/QH256_MAPPING_PROFILE.md`; the following critical rules are mandatory:

- **Cell 21 — text result available:** `TRUE` for `CAPTURED` and `CAPTURED_EMPTY`; `FALSE` for a supported candidate with no text row, regardless of whether it was skipped, unreadable, invalid UTF-8, contained NUL, or PDF extraction failed; `UNKNOWN` only for a non-candidate.
- **Cell 22 — captured text non-empty:** `TRUE`/`FALSE` when a text row exists, based on its byte length; `UNKNOWN` when no text row exists.
- **Cell 23 — PDF extractor succeeded:** only for PDF objects: `TRUE` when the `pdftotext` child exited successfully, even if output was later rejected for invalid UTF-8/NUL, per-file output limit, total frame limit, or parent pipe read failure. `FALSE` only for `PDF_EXTRACTOR_UNAVAILABLE_OR_FAILED`. `UNKNOWN` where no extractor success/failure result was produced, including a pre-extraction `SKIPPED_TOTAL_LIMIT`. Non-PDF objects remain `UNKNOWN`.
- **Cell 25 — captured text passed UTF-8 validation:** `TRUE` only for successfully captured text; `FALSE` only for `INVALID_UTF8`; `UNKNOWN` for `EMBEDDED_NUL` and other no-capture outcomes. The UTF-8 claim is separate from NUL safety and semantic correctness.
- Cells 27–30 may be `TRUE` in the final per-object QH256 index only after the sorted duplicate-path check, candidate-state order/record validation, and path Base64 roundtrip are all established. The final `INDEX.tsv` is generated after candidate serialization/reconstruction checks; mapping values must not claim those checks before they have occurred.

A corpus aggregate is the merge of per-object values and frame-operation cells through the actual QH256 v2 `qh256_merge()` function. `GUARD` in an aggregate can come from different objects carrying opposite evidence at the same cell; it is not automatically a contradiction about one object. The aggregate is lossy and is not a history, count map, proof substitute, or claim of external truth.

## 9. Retrieval and tracing

Retrieval uses exact lexical AND matching over captured text. Tokens use the documented ASCII letters/digits/underscore/hyphen rule; tokens shorter than two characters are ignored. The runtime does not perform stemming, paraphrase detection, semantic similarity, embedding search, ranking by meaning, or LLM interpretation. A query verifies its target frame before returning results. Results must include enough identifiers and hashes to trace back to a frame and source object. No hit means only that the current supported captured corpus yielded no lexical match for the query under these rules.

## 10. Failure and refusal rules

A nonzero exit or explicit refusal must not be worked around by deleting evidence or manually editing the store. Expected refusals include source/store overlap, missing source/store, invalid IDs, existing frame ID, incorrect parent or ancestry, branch ambiguity, source drift, parse failure, invalid index/status, text binding/hash mismatch, transition mismatch, QH mismatch, Gateway mismatch, manifest mismatch, and failed proof-before-state checks.

Interrupted `.stage-*` directories must be preserved and inspected. They are not a successful frame. A repair, migration, status reinterpretation, or change of schema requires a fresh versioned candidate and distinct evidence; do not “repair” an old Alpha 0 frame by replacing its status or checksums.

## 11. Platform and dependency boundary

Qualified binary scope for this candidate package is Linux x86-64 baseline using glibc symbols through `GLIBC_2.34`, a compatible ELF loader, and the direct runtime dependency `libc.so.6`. The build contract uses explicit `-march=x86-64 -mtune=generic`; local build tools are build-time dependencies, not required by the installed executable. `pdftotext` is optional. No OpenSSL, SQL database, Python runtime, network, CUDA, model daemon, or llama.cpp client is part of the runtime core.

This does not promise compatibility with all Linux systems or all future Debian releases. Reproducible-build qualification remains a separate G7 condition and is not claimed here.

## 12. Security and epistemic non-claims

The runtime is a single-writer local CLI and provides no interprocess locking protocol, digital signatures, remote trust anchor, encrypted storage, hostile-store security, or semantic-fact verification. An attacker able to rewrite the entire store and all dependent hashes may construct a new internally consistent store. Filesystem metadata can vary by platform and time. Local proof is evidence that these declared checks passed over these bytes, not a proof that the bytes are factually true or complete.

## 13. Alpha 1 acceptance criteria

G5-A may only close after all of the following are satisfied on the exact Alpha 1 package:

1. mapping unit tests pass for candidate/no-result, captured/nonempty, captured-empty, invalid UTF-8, embedded NUL, PDF failure, PDF skipped before execution, and extractor success with a rejected/skipped output;
2. end-to-end frames containing invalid UTF-8 and embedded NUL scan and subsequently `VERIFY PASS` with their distinct status strings;
3. captured-empty text survives `TEXT.tsv` serialization and verify roundtrip;
4. file and total limits leave structural records and verify successfully;
5. PDF extraction both with and without `pdftotext` follows cell 23 and status rules and does not break core operations;
6. the verifier reconstructs `text_status` from INDEX, validates its relationship to TEXT, and then recomputes identical object and aggregate QH256 values;
7. the A1 schema rejects A0 stores rather than silently interpreting them;
8. the full historical Alpha 0 regression set still passes against the A1 semantics, including proof-before-state, two-frame transition/source parity, tamper rejection, drift rejection, repeated frame ID refusal, ambiguous branch refusal, and both overlap directions;
9. source tree, package manifest, test transcript, and target host are recorded; the owner reviews the evidence before marking the gate PASS.

No release or canonization is implied by this contract. **OPEN UNTIL PASS.**
