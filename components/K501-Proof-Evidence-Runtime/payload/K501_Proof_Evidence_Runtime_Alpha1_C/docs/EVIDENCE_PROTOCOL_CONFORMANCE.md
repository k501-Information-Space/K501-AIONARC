# Evidence Protocol v0.1.0 — Conformance Mapping

**Assessment status:** bounded implementation mapping; protocol itself remains proposed and unratified.

The bundled `reference/EVIDENCE_PROTOCOL_SPECIFICATION_v0.1.0.md` and `reference/EVIDENCE_RECORD.schema.json` are retained as references. This package does not amend them or convert them to canonical status.

| Logical requirement | C recorder artifact / field | Current status | Limit / follow-up |
|---|---|---|---|
| Unique run ID | `FRAME.json:frame_id`; run directory name | Implemented | Name collisions fail closed; repeated same-second slug needs a new slug or retry after inspecting evidence |
| Start metadata | `FRAME.json`, `HOST.txt` | Implemented | Environment summary is bounded, not a full environment snapshot |
| Exact command argv | `FRAME.json:command_argv`; `COMMAND.txt`; `EVIDENCE_RECORD.json:procedure.argv` | Implemented | UTF-8 argv required so JSON does not silently reinterpret arbitrary bytes |
| Exact subject identity | `EVIDENCE_RECORD.json:subject_artifacts` | Implemented when resolvable | Subject SHA-256/size are captured before launch; post-run observation identifies a changed digest. Unresolved identities have null hash/size and a limitation |
| Working directory | `FRAME.json`, `HOST.txt`, `EVIDENCE_RECORD.json:procedure.working_directory` | Implemented | Child inherits the recorder's current working directory |
| Start/end times | `FRAME.json`, `RESULT.txt`, `EVIDENCE_RECORD.json:timing` | Implemented | Host clock is the time source; no atomic snapshot claim |
| Raw command output | `TRANSCRIPT.log` | Implemented with bounded claim | stdout/stderr merged; an I/O error yields `PARTIAL` if the exit criterion matches; separate stream capture remains open |
| Actual and expected exit | `RESULT.txt`; `EVIDENCE_RECORD.json:execution` | Implemented | Expected exit defaults to 0 and can be explicitly set; actual code returned by recorder |
| Output hashes | `SHA256SUMS.txt`; raw evidence hashes inside record | Implemented | SHA-256 provides integrity checking only, not signature or independent trust |
| Gate disposition | Explicit `NO_GATE_EFFECT_UNTIL_REVIEW` | Deliberately not performed | Gate decision remains a separate operation with its own criteria and closeout |
| Links to earlier/retry runs | `related_run_ids: []` | Not yet populated | Caller/operator must link retries in a later reviewed record; no inferred linkage |
| Git identity/status | `GIT_PRE.txt`, `GIT_POST.txt` | Partial | Captures root, HEAD, branch, status, and diff summaries when inside a Git worktree; the exact schema fields are not fully normalized into the Evidence Record |
| Human review | `reviewer: null`, `reviewed_utc: null` | Open | Recorder never invents reviewer or review time |

## Relationship to `k501-run.sh`

The C recorder intentionally reuses the run area names, directory pattern, core filenames, and protocol identifier. It adds the normalized `EVIDENCE_RECORD.json` and an explicit expected-exit value. Its `COMMAND.txt` format differs from the Bash recorder's shell-escaped line, and the details of `HOST.txt` differ. It is consequently **not byte-compatible** and must not replace the existing recorder in place until the operator approves the compatibility matrix.

This implementation is suitable for controlled side-by-side evaluation. The existing recorder remains untouched; no existing run folder is converted or rewritten.

## Check outcome versus gate

When the observed exit equals the declared expected exit and transcript capture completes, `execution.status` is `PASS` for `CONTROLLED-COMMAND-EXIT-001`. If the exit does not match, it is `FAIL`; if the exit matches but transcript capture fails, it is `PARTIAL`. That does not mean the child application passed its own tests or a project gate. The `gate_effect` is fixed at `NO_GATE_EFFECT_UNTIL_REVIEW`.
