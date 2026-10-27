# QH256 Application Mapping — K501 AIONARC Memory V3A1

**Profile:** `QH256-K501-AIONARC-MEMORY-V3A1`  
**QH core:** QH256 v2.0, unchanged  
**Status:** normative mapping profile for Alpha 1 candidate; non-canonical pending G5-A tests and owner review

## 1. No core redefinition

QH256 remains 128 two-bit cells, 256 bits, 32 bytes. Codes are `UNKNOWN=00`, `FALSE=01`, `TRUE=10`, `GUARD=11`. Serialization and `MERGE_K` are provided by the referenced QH256 v2 C core. The v3 application profile currently assigns cells 0–63; cells 64–127 remain UNKNOWN/reserved. No meaning is assigned to an unused cell merely because it exists.

A cell carries evidence according to this profile; it is not automatically a factual claim about the outside world. `UNKNOWN` means this profile does not encode a value for that observation. `FALSE` is used only where the full scan can explicitly classify the condition or absence. `GUARD` is produced by the QH256 merge algebra when both evidence planes are present; for a corpus-level aggregate it may reflect different objects' values at the same slot, not contradiction about one object.

## 2. Object cells 0–31

| Cell | Property represented | Assignment rule | Boundary / caution |
|---:|---|---|---|
| 0 | Object was observed | TRUE per object | Existence inside the declared scan scope only |
| 1 | Regular file | TRUE for regular file; FALSE for directory; otherwise UNKNOWN | Inherited v2.1-C mapping behaviour |
| 2 | Directory | TRUE for directory; FALSE for regular file; otherwise UNKNOWN | Inherited v2.1-C mapping behaviour |
| 3 | Legacy non-`other` object classification | TRUE for file/directory/symlink; FALSE for `other` | This matches the shipped v2.1-C implementation. It is **not** an independent POSIX permission/readability test |
| 4 | Size metadata field observed | TRUE | Does not mean content has semantic validity |
| 5 | Regular-file content digest available | TRUE for regular files | A failed file hash aborts scan; non-files remain UNKNOWN |
| 6 | Legacy hash-verification slot | UNKNOWN | Deliberately not repurposed; V3 records the double-read result separately in cell 20 |
| 7 | Relative path enumerated | TRUE | Means the path is bounded to this root enumeration; it does not claim arbitrary path normalization |
| 8 | Reported object size is zero | TRUE/FALSE based on observed size | Filesystem metadata; directory sizes are OS-defined |
| 9 | Reported object size is non-zero | TRUE/FALSE, inverse of cell 8 | Same caveat as cell 8 |
| 10 | Regular-file byte sequence hashed | TRUE for regular file | Symlink target bytes use their own recorded digest path, not this slot |
| 11 | V3 object record formed and checked before acceptance | TRUE in candidate mapping | A local validation result, not external truth |
| 12 | Record hash available | TRUE | Hash-domain meaning is defined in `DATA_FORMATS.md` |
| 13 | Identity stability under arbitrary moves/renames | UNKNOWN | Rename continuity is not proven or inferred |
| 14 | Legacy-supported object class | TRUE for file/directory; FALSE for symlink/other | Preserves current v2.1-C coded mapping; V3 still records these types structurally |
| 15 | Object was included in this completed scan candidate | TRUE | Candidate-only until proof/commit passes |
| 16 | Object is a symlink | TRUE/FALSE when object type is explicitly classified as symlink/non-symlink | Observed with `lstat`; link is not followed |
| 17 | Object is `other` special filesystem type | TRUE/FALSE from observed type | Socket, device and other non-regular/non-directory/non-symlink cases remain structural only |
| 18 | Relative path was collected inside the declared root | TRUE | Scan scope checked before enumeration |
| 19 | Content/link-target digest available | TRUE when a digest exists | Directories typically remain UNKNOWN |
| 20 | Regular bytes or symlink target were hashed twice and matched | TRUE for files and symlinks after scan checks | Not an external storage durability assertion |
| 21 | Text-extraction result available | TRUE for `CAPTURED`/`CAPTURED_EMPTY`; FALSE for any supported candidate with no `TEXT.tsv` row; UNKNOWN for non-candidate | A failed/skipped capture is a known non-result; no-candidate is not failure |
| 22 | Captured text is non-empty | TRUE/FALSE if text capture exists; UNKNOWN otherwise | Empty text can be faithfully captured but is not searchable |
| 23 | PDF extractor process succeeded | TRUE when `pdftotext` exited successfully, including later rejected/skipped output; FALSE only on `PDF_EXTRACTOR_UNAVAILABLE_OR_FAILED`; UNKNOWN if the extractor was not run/no determinate outcome; UNKNOWN for non-PDF | `PDF_OUTPUT_FILE_LIMIT`, `PDF_CAPTURE_SKIPPED_TOTAL_LIMIT`, `PDF_CAPTURE_READ_FAILED`, `INVALID_UTF8`, `EMBEDDED_NUL` prove child success |
| 24 | File has a supported text-capture extension/type | TRUE/FALSE for regular files; UNKNOWN for other types | Candidate list in source/manual is the actual implementation boundary |
| 25 | Captured text passed UTF-8 validation | TRUE for `CAPTURED`/`CAPTURED_EMPTY`; FALSE only for `INVALID_UTF8`; UNKNOWN for `EMBEDDED_NUL` and all other no-capture outcomes | UTF-8 validity is distinct from NUL safety and semantic correctness |
| 26 | Source digest was rechecked after text extraction | TRUE when text capture succeeds | Bind is to the observed byte digest, not semantic correctness |
| 27 | Relative path uniqueness checked | TRUE after sorted duplicate-path check | Limited to a successful scan result |
| 28 | Deterministic path order checked | TRUE after sorting and candidate comparison | Bytewise C-string ordering for valid filesystem path byte strings; NUL cannot occur in a path |
| 29 | Object ID and record hash recomputed against candidate state | TRUE after candidate roundtrip validation | Proof gate covers this check |
| 30 | Relative path survived serialized Base64 roundtrip | TRUE after candidate/index checks | Base64 is transport escaping, not encryption |
| 31 | Symlink was recorded as a symlink without following it | TRUE for symlink objects; UNKNOWN otherwise | This is a local scanner behaviour check |

## 3. Transition and proof cells 32–47

These properties describe the *frame operation*, and are merged into the same aggregate snapshot as the object cells. They are not per-object claims. In Alpha 1 the final `INDEX.tsv` and its per-object QH256 values are written only after candidate state/path reconstruction checks, so cells 27–30 are not asserted prematurely.

| Cell | Property | Assignment |
|---:|---|---|
| 32 | At least one object added | TRUE if `added > 0`, else FALSE after full transition classification |
| 33 | At least one object removed | TRUE if `removed > 0`, else FALSE |
| 34 | At least one content change | TRUE if `content_changed > 0`, else FALSE |
| 35 | At least one metadata-only change | TRUE if `metadata_changed > 0`, else FALSE |
| 36 | At least one unchanged object | TRUE if `unchanged > 0`, else FALSE |
| 37 | Initial-frame transition | TRUE if all current objects are added and no other event exists; otherwise FALSE |
| 38 | Subsequent transition | TRUE when transition is not initial; otherwise FALSE |
| 39 | Parent frame validated | TRUE when a parent exists and its chain validates; UNKNOWN for initial frame |
| 40 | Candidate state parsed, rehashed and object-count checked | TRUE after validation |
| 41 | Captured text was re-read, hash-checked and bound to source records | TRUE after validation |
| 42 | Object IDs and record hashes recomputed against serialized candidate | TRUE after validation |
| 43 | Derived index rows and per-object QH256 values validated | TRUE after validation |
| 44 | Gateway header and entry set validated | TRUE after validation |
| 45 | QH256 core test vector/property checks passed | TRUE if embedded core checks pass |
| 46 | Candidate frame ID was absent before operation | TRUE before staging begins |
| 47 | Source/store scopes are disjoint | TRUE after canonical-path overlap check |

## 4. Whole-inventory cells 48–63

These cells summarize whether a feature was observed anywhere in the complete inventory. TRUE means “one or more observed”, FALSE means “none observed in the completed full scan”. They do not count how many; counts remain available from the object inventory where relevant.

| Cell | Property | TRUE condition | FALSE condition |
|---:|---|---|---|
| 48 | DOCX files present | At least one `.docx` regular file | None observed |
| 49 | Markdown/plain-text documents present | At least one `.md`, `.txt` or `.markdown` regular file | None observed |
| 50 | Configuration files present | At least one `.json`, `.yaml`, `.yml`, `.toml`, `.ini`, `.conf` or `.cfg` regular file | None observed |
| 51 | Source-code files present | At least one supported C/C++/Python/Shell/SQL/JS/TS source extension | None observed |
| 52 | PDF files present | At least one `.pdf` regular file, whether or not extraction succeeds | None observed |
| 53 | Zero-length regular files present | At least one zero-byte regular file | None observed |
| 54 | Hidden path present | At least one path component begins with `.` | None observed |
| 55 | Non-ASCII path present | At least one path byte is 128 or higher | None observed |
| 56 | Large file present | At least one regular file exceeds 8 MiB | None observed |
| 57 | `other` special object present | At least one object has type `other` | None observed |
| 58 | Text candidate skipped/failed | At least one candidate lacks a successful text capture | None observed |
| 59 | Extensionless regular file present | At least one regular-file basename contains no dot | None observed |
| 60 | Duplicate content digest present | At least two inventory objects share a regular-file content digest | No duplicate digest detected |
| 61 | Two or more regular files present | Regular-file count is at least two | Fewer than two |
| 62 | Directory present | At least one directory observed, typically including root `.` | None observed |
| 63 | Symlink present | At least one symlink observed | None observed |

## 5. Aggregate semantics

The per-object QH256 values and operation QH256 value are folded using the actual QH256 v2 `qh256_merge()` function. This operation is a compact state/evidence summary, not a count map, not a complete state archive, and not a semantic classifier. The complete per-object mapping remains inspectable as the `object_qh256` field in `INDEX.tsv`; history and provenance remain in frame records and transitions.

The index rows are derived and carry the mapping-profile name at the top. Changing a mapping later requires a new versioned profile and an explicit compatibility decision; it must not silently redefine this profile or QH256 v2.

## 6. Alpha 1 text status taxonomy and reconstruction rule

Normative detail is defined by `MEMORY_CONTRACT_V1.0.md`, Sections 7–8. Status vocabulary: `NOT_CANDIDATE`, `CAPTURED`, `CAPTURED_EMPTY`, `SKIPPED_TOTAL_LIMIT`, `SKIPPED_FILE_LIMIT`, `READ_FAILED`, `INVALID_UTF8`, `EMBEDDED_NUL`, `PDF_EXTRACTOR_UNAVAILABLE_OR_FAILED`, `PDF_OUTPUT_FILE_LIMIT`, `PDF_CAPTURE_SKIPPED_TOTAL_LIMIT`, `PDF_CAPTURE_READ_FAILED`. `NOT_UTF8_TEXT` is not an Alpha 1 status.

Verifier requirement: validate `INDEX.tsv` status against object type/path and any bound `TEXT.tsv` entry; restore the validated status to the reconstructed object; then calculate the per-object QH256 value. Status is derived and not a field in `STATE.tsv`, but it is part of the evidence needed to deterministically reconstruct the mapping.
