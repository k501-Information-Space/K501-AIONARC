# Architecture — K501 Proof and Evidence Runtime Alpha 1

## Modules

| Module | Implementation | Responsibility | Explicit non-responsibility |
|---|---|---|---|
| Controlled Run Recorder | `src/k501_proof_evidence.c` | Validate invocation, create unique run directory, capture metadata and merged transcript, record actual/expected exit, hash evidence | Gate acceptance or interpretation of command meaning |
| SHA-256 | `src/sha256.c` | Content digests for subject and evidence files | Authentication, digital signature, or protection against a writer who can replace all files and checksums |
| QH256 v2 | `src/qh256.c` | Existing state algebra, serialization, cell access, `MERGE_K` | Evidence-status policy or application mapping not defined by a separate contract |
| Test Harness | `tests/test_runtime.c` | C-based integration tests of CLI, output files, expected-failure path, checksum manifest, optional runtime smoke test | Production runtime operation |
| AIONARC Runtime | External executable provided at invocation time | Source observation, frames, mapping and retrieval according to its own contract | Evidence recorder does not inspect its private internal state; it captures the explicit invocation and process output |

## Operation order

1. Validate area, slug, expected exit code, executable argument vector and workspace root.
2. Create a new run directory under `runs/<area>/`; refuse a reused identifier.
3. Record `FRAME.json`, `COMMAND.txt`, `HOST.txt`, `GIT_PRE.txt`, and initial `STATUS.txt`.
4. Hash the child executable before launch when its identity resolves to a readable file.
5. Execute the exact argv vector without an inserted shell; merge child stdout/stderr into `TRANSCRIPT.log`.
6. Capture post-execution Git state, end time, actual exit code and final process status.
7. Re-hash the child executable to identify observed digest change across the run.
8. Emit `EVIDENCE_RECORD.json` linking the invocation to hashes of `COMMAND.txt`, `TRANSCRIPT.log`, and `RESULT.txt`.
9. Emit `SHA256SUMS.txt` for the nine other evidence files.

If transcript capture encounters an I/O error, the Evidence Record is `PARTIAL` when the observed child exit otherwise matches the declared criterion; if the exit criterion itself fails, the record is `FAIL`. The tool returns recorder error `73` on capture failure. The child exit remains recorded in `RESULT.txt`.

If closeout fails, the created run directory is retained. The recorder does not delete or reuse partial evidence directories. Partial directories must be inspected and assessed; they are not implied PASS.

## Conformance relation

The C implementation uses the proposed Evidence Protocol v0.1.0 record schema as a reference. It does not ratify that protocol. The current record represents the specific check `CONTROLLED-COMMAND-EXIT-001`. The record's `execution.status` concerns the expected process-exit criterion; `outcome.gate_effect` remains `NO_GATE_EFFECT_UNTIL_REVIEW`.

## QH256 boundary

The QH256 source included here is copied from the tested Alpha 1 Runtime package without modification. Its implementation uses two explicit 128-bit planes represented by four `uint64_t` words, with explicit little-endian serialization to 32 bytes. The `self-test` checks the four cell states, `MERGE_K(TRUE,FALSE)=GUARD`, serialization roundtrip, bounds rejection, and SHA-256 known vectors. These tests do not imply all possible QH256 states were enumerated.

No evidence status is automatically written into QH256. QH256 mapping for a particular runtime application remains the responsibility of that versioned mapping profile.

## Known limits

- stdout and stderr are merged into one transcript.
- Environment capture is bounded and does not snapshot every environment variable.
- Process exit status is not process meaning, content truth, or gate disposition.
- There is no signed manifest, remote witness, encryption, multi-writer coordination contract, timeout policy, or hostile-store defense.
- Run directory naming collisions fail closed instead of retrying with a silent suffix.
- The C artifact has been compiled and exercised in the build environment; the user's Debian host workflow remains unverified.
