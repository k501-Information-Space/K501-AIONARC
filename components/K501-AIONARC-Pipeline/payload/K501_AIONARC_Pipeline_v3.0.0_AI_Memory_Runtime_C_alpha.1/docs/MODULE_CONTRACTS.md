# V3A1 module contracts

## 1. C CLI / orchestrating process

**Purpose:** explicit, serial terminal operations over a local store.  
**Inputs:** command arguments and local source/store paths.  
**Outputs:** deterministic data artifacts except timestamp metadata, plus human-readable terminal messages.  
**Side effects:** `init` creates a new store; `scan` appends a new frame under the store; other commands are intended read-only.  
**Failure rule:** nonzero exit and a concrete error; do not infer success from partial output.  
**Not allowed:** implicit Git changes, source mutation, auto-deletion of failed stages, automatic selection of ambiguous branch tips, semantic interpretation, model execution.

## 2. QH256 core adapter

**Purpose:** use the existing QH256 v2 state algebra and wire representation.  
**Inputs:** explicit cell state assignments from the versioned V3 profile.  
**Outputs:** per-object QH256 values and one aggregate snapshot.  
**Validation:** `qh256-test`, serialization roundtrip, every index row's object QH recomputation and aggregate reconstruction during frame verification.  
**Boundary:** no QH256 core function or serialization is redefined by Alpha 1.

## 3. Filesystem observer

**Purpose:** detect filesystem objects in a declared root without changing the source.  
**Inputs:** existing source directory, root ID, isolated store.  
**Outputs:** sorted object set and digests.  
**Failure modes:** permission failure, missing path, race/detected modification, object/path safety limit, source/store overlap.  
**No-follow rule:** uses `lstat`; symlinks are observed as links and not traversed.  
**Limit:** not an immutable filesystem snapshot; it detects selected races by metadata and repeated hashing but cannot establish a globally atomic filesystem snapshot under all possible concurrent adversarial changes.

## 4. State / transition engine

**Purpose:** persist each observation as a new versioned frame and derive structural changes from a validated predecessor.  
**Inputs:** current observed objects and optionally a verified parent frame.  
**Outputs:** candidate state, transition event set, state digest, chain digest.  
**Safety:** repeated frame ID refused; parent root/sequence/state binding checked; no rename continuity guessed.

## 5. Text capture

**Purpose:** expose exact UTF-8 text to the search interface while binding it to observed source bytes.  
**Inputs:** regular files with supported extensions and PDFs where `pdftotext` is present.  
**Outputs:** `TEXT.tsv`, extractor IDs, text hashes and explicit status.  
**Limits:** max text per file 16 MiB; aggregate text cap 256 MiB; not a general Office/document parser, no OCR, no semantic summary.

## 6. Derived index

**Purpose:** deterministic navigation from state object to type, source/text digest, retrieval status, path and per-object QH256.  
**Inputs:** validated candidate records and text-capture results.  
**Outputs:** `INDEX.tsv`.  
**Validation:** exact row order, fields, path encoding, per-object QH256 and text status acceptance are tested before proof and during verify.  
**Status:** derived and replaceable only by a newly framed candidate; never canonical source.

## 7. Gateway

**Purpose:** provide a small human/machine orientation map to the frame artifacts and top-level objects.  
**Inputs:** validated frame metadata and sorted object set.  
**Outputs:** `GATEWAY.tsv`.  
**Validation:** exact header fields and top-level entries reconstructed before proof and during verify.  
**Non-purpose:** not a semantic ontology, permission system or independent source of truth.

## 8. Proof gate / commit

**Purpose:** enforce the Proof Before State contract declared in `PROOF_BEFORE_STATE_CONTRACT.md`.  
**Inputs:** staged candidate bytes and their derived artefacts.  
**Outputs:** durable `PROOF.tsv`, then committed `STATE.tsv` and frame directory.  
**Failure:** stage remains visible and unpromoted.  
**Limit:** no digital signature or adversarial whole-store protection.

## 9. Retrieval / trace API via CLI

**Purpose:** search captured text using case-folded ASCII token substrings with AND matching; produce candidate passages with source hashes and object IDs.  
**Commands:** `query`, `trace`, `history`, `diff`, `verify-source`.  
**Boundary:** lexical retrieval is not semantic relevance. It returns excerpts as retrieval results; it does not generate claims or decide source truth. Every query first verifies its selected frame.

## 10. Future model adapter (not implemented)

A later adapter may submit retrieved text to llama.cpp after a separate interface and admission contract is approved. It must be read-only against canonical data; bind prompts/results to exact frame/object/content/text hashes; reject stale/unverified frames; treat output as `DERIVED_CANDIDATE`; keep model content from triggering tools or file mutations; and never write model interpretations directly into `STATE.tsv` or QH256 core cells without a separately versioned mapping and explicit gate.
