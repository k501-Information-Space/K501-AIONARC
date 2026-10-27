# Alpha 1 Memory Contract and Package — Self-Review Report

**Time anchor:** Unix `1792121537` (2026-10-16 03:32:17 UTC / 05:32:17 CEST).  
**Candidate:** `3.0.0-alpha.1`.  
**State:** review performed in a separate working copy; NON-CANONICAL; operator test series pending.

## 1. Inputs reconstructed for this review

The specification pass consulted the K501 canonical system specification (2026-09-27), Pure Logic reference package, K501 runtime-architecture draft (2026-10-12), current V3 gate roadmap (2026-10-15), latest supplied K501 state/evidence summary, the Alpha 0 runtime package, its Mapping Profile and Proof Before State contract, and current public QH256 root-level license/README metadata. These inputs were treated as versioned project references, not as proof that every historical statement is still operationally true.

## 2. Alpha 0 defects used as regression requirements

1. Cell 21 left a supported candidate without text capture as UNKNOWN instead of FALSE.
2. The old `NOT_UTF8_TEXT` status conflated malformed UTF-8 with a UTF-8-valid embedded NUL.
3. The verifier reconstructed captured text but did not restore validated `text_status` before recomputing object QH256 values.
4. A zero-length final Base64 field could be lost by tab tokenization, so `CAPTURED_EMPTY` was not reliably round-trippable.
5. PDF child-exit success was conflated with eventual text acceptance in the old mapping.
6. Final per-object index mapping was created before all candidate-state and path-serialization checks were complete.

The owner-provided Alpha 0 diagnostic runs reproduced `VERIFY FAIL` for invalid UTF-8 and embedded-NUL fixtures while a file-limit status verified successfully. Alpha 0 itself has not been edited by this package preparation.

## 3. Alpha 1 corrections reviewed

- New `V3A1` store/frame/text/index/proof namespaces and a new mapping-profile ID prevent silent reuse of V3A0 frames.
- QH256 v2 algebra and 32-byte serialization are unchanged.
- Object identity and object record formulas remain unchanged by explicit design; the frame chain domain is versioned to A1.
- Status vocabulary distinguishes `INVALID_UTF8`, `EMBEDDED_NUL`, successful capture, empty capture, skipped limits, read errors and PDF adapter outcomes.
- Cell 21, 22, 23 and 25 rules match `docs/MEMORY_CONTRACT_V1.0.md` and `docs/QH256_MAPPING_PROFILE.md`.
- The verifier validates/indexes `text_status`, restores it and recomputes per-object and aggregate QH256 values.
- Candidate state/text/path reconstruction precedes final `INDEX.tsv` creation; proof is flushed before the state candidate is renamed to `STATE.tsv`; the manifest is then generated over the completed artifact names before frame-directory promotion.
- Tests explicitly refuse Alpha 0 stores, test both source/store overlap directions, and exercise invalid UTF-8, NUL, empty text, limits and optional PDF operation.
- SHA-256 implementation was rewritten and tested against known vectors; the archived v2.1-C package is not redistributed.

## 4. Package-local test results

After the last code and test edits, the build and complete suite returned success:

- `make clean all` — PASS.
- `qh256-test` — PASS.
- `sha256-test` — PASS.
- `mapping-test` — PASS.
- `make test` — `K501 V3A1 TEST SUITE: PASS`.
- An additional full suite run with AddressSanitizer/UndefinedBehaviorSanitizer and leak detection — PASS, no sanitizer diagnostics.

The exact delivered archive must still be verified by its sidecar hash, unpacked into a fresh directory, and exercised through the owner's `tools/k501-run.sh`. A test passing in this package-building environment is not a claim that the separate operator host has passed.

## 5. License/self-review boundary

The package adopts the owner's requested prospective asset-class framework: MIT for code, CC BY 4.0 for authored docs/specs/conduct where rights are held, and separate upstream terms for third-party materials. The transition notice only supersedes previous internal K501 policy/status notices for prospective packages; it does not purge history or purport to revoke earlier public grants. The current public QH256 repository presents an MIT root license, but current online file-level source matches could not all be resolved during this pass. The package's rights-holder declaration records the owner's instruction and is not digitally signed or independently adjudicated.

Accordingly, this review does **not** claim G5-C legal clearance. The owner must confirm authority over each MIT/CC-BY file class before public redistribution. Names, marks, personal data, private corpus files and external tools not bundled here are not licensed by implication.

## 6. Gate disposition

- `G5-A`: OPEN pending operator-side tests and owner review of contract/evidence.
- `G5-B`: PASS for the explicitly qualified Linux x86-64/glibc scope documented in the prior controlled run; the delivered A1 package still needs operator-host recheck.
- `G5-C`: OPEN pending rights-holder file-scope confirmation and provenance review.
- `G7`: OPEN; reproducible-build claim not made.
- Canonical release/publication: NOT APPROVED.

**Conclusion:** Alpha 1 is a materially corrected, test-passing engineering candidate for controlled unpack and further verification. It is not yet canonical and must not be treated as released or independently validated.
