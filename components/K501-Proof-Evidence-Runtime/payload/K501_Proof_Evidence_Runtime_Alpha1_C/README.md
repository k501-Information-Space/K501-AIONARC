# K501 Information Space — Proof and Evidence Runtime Alpha 1 (C11)

**Package identity:** `K501_Proof_Evidence_Runtime_Alpha1_C`  
**Implementation:** native C11  
**Version:** `0.1.0-alpha.1`  
**Status:** engineering candidate; not ratified as canonical; not a release acceptance  
**Scope:** controlled process execution, run capture, artifact identity, normalized evidence records, and a separately tested QH256 v2 core.

## What this is

The K501 Information Space is the coherent, reconstructable structure of identities, contracts, objects, state, time, provenance, evidence, and relations. It is not itself an executable, LLM, shell script, or monolithic runtime.

This package gives one bounded operational part of that Information Space a native C11 implementation: **K501 Proof and Evidence Runtime Alpha 1**. It records explicit command executions in the established `runs/<area>/<run-id>/` namespace and emits raw capture plus a normalized Evidence Record. The existing K501 AIONARC Runtime remains a separate process and separate operation layer. The recorder invokes it only when the operator supplies its executable and arguments.

The existing QH256 v2 source is included as a separate core module. Its algebra and wire representation are not redefined. The recorder does not automatically encode `PASS`, `FAIL`, or other evidence statuses into QH256 cells. Such a mapping requires its own approved contract.

## Operations

```text
k501-proof-evidence version
k501-proof-evidence self-test
k501-proof-evidence identity <file>
k501-proof-evidence run <workspace-root> <area> <slug> [--expect-exit N] -- <command> [arg ...]
```

The default expected child exit code is `0`. `--expect-exit N` makes a different expected process exit explicit. The recorder returns the actual child exit code when capture completes; if capture I/O fails, it returns `73` while preserving the observed child result in the retained evidence. The record reports `PASS` when the declared exit criterion matches, `FAIL` when it does not, and `PARTIAL` when the exit criterion matches but transcript capture fails. A matching exit criterion is **not** a project gate PASS.

The wrapper passes the declared argv vector to `execvp`; it does not insert a shell. All arguments are recorded, so do not pass credentials, passwords, access tokens, private keys, or secret-bearing material.

## Build and test

Requires a POSIX/Linux environment, C11 compiler and GNU Make.

```sh
make clean all
make test
```

To include an explicit smoke test against an existing K501 Runtime binary:

```sh
make test RUNTIME_BIN="/path/to/k501-memory-linux-x86_64"
```

The optional integration test runs the supplied executable as a separate process using `qh256-test` and validates its captured evidence. It does not modify the runtime binary or its source.

## Evidence output

A run creates a unique directory with restrictive permissions under:

`<workspace-root>/runs/<area>/<unix-epoch>_<UTC timestamp>_<slug>/`

The output includes `FRAME.json`, `COMMAND.txt`, `HOST.txt`, `GIT_PRE.txt`, `GIT_POST.txt`, `TRANSCRIPT.log`, `RESULT.txt`, `STATUS.txt`, `EVIDENCE_RECORD.json`, and `SHA256SUMS.txt`.

`SHA256SUMS.txt` hashes all nine other files and excludes itself. The Evidence Record conforms to the structure of the included `EVIDENCE_RECORD.schema.json` in the exercised cases. The schema and Evidence Protocol v0.1.0 are reference materials with **proposed / not ratified** status; packaging them or producing a schema-valid record does not ratify the protocol.

## Operational boundaries

- `FRAME.json` is start metadata; `RESULT.txt` and `STATUS.txt` are the final process result.
- `EVIDENCE_RECORD.json` evaluates only the declared process-exit criterion. Its `gate_effect` is always `NO_GATE_EFFECT_UNTIL_REVIEW`.
- The recorder preserves prior run directories and refuses to reuse an existing run identifier.
- Source observation and state creation remain the K501 Runtime's responsibility.
- QH256 remains the state algebra; the evidence module does not define a new state mapping.
- No LLM, embedding retrieval, semantic interpretation, gate closure, signature service, encryption, remote trust anchor, or hostile-store protection is included.
- Output capture currently merges stdout and stderr in `TRANSCRIPT.log`; it does not preserve them as separate streams. A transcript I/O failure is recorded as `PARTIAL` when the exit criterion otherwise matches, and the run is not presented as a full PASS.
- No claim is made that the target operator host has been tested. The host-side integration gate remains open until the operator repeats validation through the approved workflow.

See `docs/K501_INFORMATION_SPACE.md`, `docs/ARCHITECTURE.md`, `docs/EVIDENCE_PROTOCOL_CONFORMANCE.md`, and `docs/OPERATIONS.md`.
