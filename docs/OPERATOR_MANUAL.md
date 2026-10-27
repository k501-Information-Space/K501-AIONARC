# Operator Manual — K501 Integrated Pipeline & Proof Runtime 3.5.0

**State:** candidate instructions for owner evaluation. Test only in a new, isolated directory until this aggregate is accepted.

## 1. Requirements and safe use

- POSIX/Linux, GNU Make, a C11 compiler, Python 3 and `sha256sum` for build/verification.
- AIONARC's qualified runtime binary targets Linux x86-64 and requires GLIBC symbols through 2.34. This is not a universal Linux portability promise.
- `pdftotext` is optional. Without it, structural scanning and non-PDF operations remain available; PDF extraction outcomes follow the component contract.
- Use a new source fixture and a new empty store. Source and store must not overlap. Do not test first against a canonical or private corpus.
- The recorder captures command arguments. Never pass credentials, tokens, passwords or private keys as argv.

## 2. Build and tests

From the aggregate root:

```sh
make clean
make all
make test
make verify
```

`make all` rebuilds both source packages and stages `dist/linux-x86_64/k501-memory` and `dist/linux-x86_64/k501-proof-evidence`. `make test` runs the AIONARC self-tests and regression suite, then the Proof/Evidence suite with the exact just-built AIONARC binary passed as `RUNTIME_BIN`. `make verify` also checks both component manifests and the aggregate schemas, source identity, index, manifest and checksums.

For an operator acceptance run, invoke each final command through the approved K501 run recorder and preserve its entire run directory. The standard wrapper's own exit status remains process evidence, not a gate disposition.

## 3. Isolated AIONARC quickstart

Run from the aggregate root. The commands create a disposable directory under the system temporary directory. Review the generated files before removing it.

```sh
tmp=$(mktemp -d "${TMPDIR:-/tmp}/k501-v35-aionarc.XXXXXX")
mkdir "$tmp/source"
printf 'proof before state\n' > "$tmp/source/README.md"
dist/linux-x86_64/k501-memory init "$tmp/store"
dist/linux-x86_64/k501-memory scan "$tmp/source" "$tmp/store" k501-v35-test frame-001 none
dist/linux-x86_64/k501-memory verify "$tmp/store" frame-001
dist/linux-x86_64/k501-memory query "$tmp/store" latest "proof before state"
```

The scan only reads the declared source and writes a separate store. For subsequent frames, use a new frame ID and an explicit valid parent. The store refuses repeated IDs, ambiguous branch tips, legacy schema mixing and overlapping source/store paths. Preserve a failed/staged frame and inspect diagnostics; do not delete evidence to make a later run appear clean.

## 4. Record a child operation

Use a separate evidence workspace so the recorder's output cannot overlap AIONARC's source or store:

```sh
tmp=$(mktemp -d "${TMPDIR:-/tmp}/k501-v35-recorder.XXXXXX")
mkdir "$tmp/source" "$tmp/evidence-workspace"
printf 'bounded recorder smoke test\n' > "$tmp/source/input.txt"
recorder="$PWD/dist/linux-x86_64/k501-proof-evidence"
memory="$PWD/dist/linux-x86_64/k501-memory"
"$recorder" run "$tmp/evidence-workspace" system recorder-smoke -- /usr/bin/wc -c "$tmp/source/input.txt"
```

Inspect the new run's `FRAME.json`, `COMMAND.txt`, `HOST.txt`, `TRANSCRIPT.log`, `RESULT.txt`, `EVIDENCE_RECORD.json`, `STATUS.txt` and `SHA256SUMS.txt`. Validate the record against the exact bundled Evidence Record schema. The record's `gate_effect` is `NO_GATE_EFFECT_UNTIL_REVIEW`.

To repeat the bounded pipeline integration manually:

```sh
"$recorder" run "$tmp/evidence-workspace" pipeline aionarc-qh256-smoke -- "$memory" qh256-test
```

Use an absolute executable path in formal evidence so identity resolution is unambiguous. The aggregate test harness passes the exact absolute path automatically.

## 5. AIONARC operations

The common flow is `init` → `scan` → `verify` → `query` → `trace/history`. For changes, scan a new frame with the explicit previous frame ID, then verify/diff and verify-source. See the preserved [AIONARC Operator Manual](../components/K501-AIONARC-Pipeline/payload/K501_AIONARC_Pipeline_v3.0.0_AI_Memory_Runtime_C_alpha.1/docs/OPERATOR_MANUAL.md) and [Quick Reference](../components/K501-AIONARC-Pipeline/payload/K501_AIONARC_Pipeline_v3.0.0_AI_Memory_Runtime_C_alpha.1/docs/OPERATOR_QUICK_REFERENCE.md) for full command grammar and recovery cases.

Retrieval is exact lexical matching, not semantic search. `STATE.tsv`, `TEXT.tsv`, `INDEX.tsv`, `PROOF.tsv`, transition records and QH256 state each have distinct roles. Treat the index as derived and validate it before trusting reconstructed values.

## 6. Evidence and integrity review

Use `make verify` and independently inspect the output. Verify the root `SHA256SUMS.txt` and its sidecar, then each component's own manifest. Keep run artifacts append-only. A zero process exit, valid hash, schema-valid record or `PASS` in a component report does not by itself mean the aggregate is legally distributable, accepted on another host, canonical, or released.

## 7. Limits and stop conditions

Stop if source identity changes, a checksum differs, a required tool is absent, a command writes outside its declared output area, a test fails, a child binary is not the one built from this tree, or rights/authority is unclear. Preserve the run and record a new attempt after correction. Do not overwrite the component recorder, AIONARC store, historical runs, old frame IDs or component license records.

No automatic cleanup is performed. When evaluation is complete, inspect and manually remove only the disposable test directory you created, if desired.
