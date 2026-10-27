# Alpha 1 Decision Log

## ADR-A1-001 — Preserve Alpha 0

**Decision:** Alpha 0 stays unchanged as historical evidence. Alpha 1 is a separate package with new schema identifiers and a new mapping-profile identifier.  
**Reason:** discovered mapping/verification defects must be corrected without rewriting the history that exposed them.

## ADR-A1-002 — Status is a derived but required reconstruction input

**Decision:** `text_status` remains outside structural `STATE.tsv`, but Alpha 1 must validate and restore it from `INDEX.tsv` before computing per-object QH256 on verification.  
**Reason:** mapping cannot deterministically reconstruct from `STATE.tsv` and `TEXT.tsv` alone when a text candidate yielded no text.

## ADR-A1-003 — Separate invalid UTF-8 from NUL

**Decision:** `INVALID_UTF8` is used only when UTF-8 validation fails; UTF-8-valid text containing NUL gets `EMBEDDED_NUL`.  
**Reason:** these are different byte properties and cell 25 only represents UTF-8 validation.

## ADR-A1-004 — PDF process success differs from text acceptance

**Decision:** PDF cell 23 records successful `pdftotext` child completion, not whether output was ultimately accepted into `TEXT.tsv`. Output-limit, NUL, or UTF-8 rejection may coexist with extractor success.  
**Reason:** the evidence planes must represent the predicate they name.

## ADR-A1-005 — Final index is generated after candidate reconstruction

**Decision:** write the definitive `INDEX.tsv` and per-object QH256 values only after candidate state is re-read, object/record hashes are recomputed, text capture roundtrip is validated, and the Base64 path roundtrip is established.  
**Reason:** cells 11 and 27–30 must not assert validation work before it occurs.

## ADR-A1-006 — No inherited v2.1-C archive in Alpha 1

**Decision:** do not redistribute the prior v2.1-C ZIP within the A1 package. Record its historical SHA-256 and the four-file byte-match fact as provenance only. The A1 runtime is built from the explicit source tree and its own file-level license map.  
**Reason:** keep the new package's license scope clear and avoid bundling an old archive whose own release-level license status was unresolved.

## ADR-A1-007 — License policy is split by asset class

**Decision:** MIT for K501 source/build/test code; CC BY 4.0 for K501-authored documentation, contract, data-format specification and code of conduct; explicit upstream notices for anything third-party. Raw corpus/evidence rights are not granted by this software package.  
**Reason:** code, authored prose, source inputs, and third-party components are different rights boundaries.
