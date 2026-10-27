# Alpha 1 Data Formats

All durable frame artifacts use the `V3A1` schema namespace, UTF-8 metadata and LF line endings unless a file is specifically a byte payload. Alpha 1 stores are deliberately incompatible with Alpha 0 store-marker/schema magic and must be initialized as new stores.

| File | Role | Data boundary |
|---|---|---|
| `STORE_FORMAT.txt` | Identifies the store schema and creator version | Non-canonical store metadata |
| `FRAME.tsv` | Frame/root/sequence/parent, state and chain digests, object/event counts, profile | Frame binding metadata |
| `STATE.tsv` | Sorted structural object records: object ID, record hash, content digest, type, mode, size, mtime, Base64 relative path | Structural observation state; no captured text/status |
| `TEXT.tsv` | Captured text rows bound to object ID, source digest, text digest, extractor, path, length and Base64 bytes | Derived source-bound text |
| `INDEX.tsv` | One row per object with identities, optional text references, searchability, Base64 path, normative status and per-object QH256 | Derived, validated mapping projection |
| `TRANSITION.tsv` | Parent relation, event counts and deterministic event rows | Derived structural transition |
| `GATEWAY.tsv` | Verified frame pointers and top-level object entries | Orientation only |
| `QH256.hex` | QH256 v2 32-byte state, encoded as 64 hex chars, profile header | Aggregate state; not SHA-256 and not history |
| `PROOF.tsv` | PASS status, contract/frame/digest binding, performed checks and non-claim | Local proof record |
| `MANIFEST.sha256` | Hash list for eight frame artifacts, excluding itself | Byte integrity manifest |

## Base64 and empty text

Paths are Base64-encoded to preserve tabs, spaces and non-ASCII bytes in tab-separated rows. Base64 is not encryption. `TEXT.tsv` always includes eight tab-separated fields per record; for `CAPTURED_EMPTY`, field 8 is intentionally empty. Parsers must preserve a trailing empty field. A zero-byte text payload still has SHA-256 of the empty byte sequence and a `TEXT.tsv` row; it is not searchable.

## Status field

The `INDEX.tsv` status vocabulary and mapping are normative in `MEMORY_CONTRACT_V1.0.md`. The status field is not copied to structural `STATE.tsv`. During verification, `INDEX.tsv` statuses are parsed, constrained, restored to the reconstructed in-memory objects, and used before QH256 recomputation. An unknown or contradictory status is a verification failure.

## Hash domain separation

`object_id`, `record_hash`, `content_sha256`, `text_sha256`, `state_sha256`, `chain_sha256`, QH256 state bytes and artifact SHA-256 values are not interchangeable. Alpha 1 retains the prior object identity and object-record formulas while using new frame/store/mapping schema IDs and an Alpha 1 chain-hash domain.
