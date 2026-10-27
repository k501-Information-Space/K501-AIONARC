# Operator Guide

## Build

```sh
make clean all
make test
```

Build output is `build/k501-proof-evidence`. The prebuilt candidate is in `bin/k501-proof-evidence-linux-x86_64`.

## Inspect identity

```sh
./build/k501-proof-evidence version
./build/k501-proof-evidence self-test
./build/k501-proof-evidence identity ./build/k501-proof-evidence
```

## Capture an ordinary operation

Use an isolated workspace first, never the canonical source tree for initial tests:

```sh
mkdir -p "$HOME/K501_PROOF_EVIDENCE_A1_TEST"
printf 'evidence runtime smoke test\n' > "$HOME/K501_PROOF_EVIDENCE_A1_TEST/input.txt"
./build/k501-proof-evidence run "$HOME/K501_PROOF_EVIDENCE_A1_TEST" system c-runtime-smoke -- /usr/bin/wc -c "$HOME/K501_PROOF_EVIDENCE_A1_TEST/input.txt"
```

## Capture the existing K501 Runtime

```sh
./build/k501-proof-evidence run "$HOME/K501_PROOF_EVIDENCE_A1_TEST" pipeline runtime-alpha1-qh256 -- /absolute/path/to/k501-memory-linux-x86_64 qh256-test
```

The recorder does not absorb the child executable into its own operations. It records the exact child argv and its process evidence. Replace the path with the exact Runtime artifact approved for the test and verify its SHA-256.

## Expected nonzero child exit

```sh
./build/k501-proof-evidence run "$HOME/K501_PROOF_EVIDENCE_A1_TEST" system expected-exit-23 --expect-exit 23 -- /absolute/path/to/test-program
```

The example is valid only if the selected test program is intentionally specified to exit `23`. A nonzero exit without declared expected behavior is not automatically a PASS.

## Inspect results

Check all files in the new run directory. Verify the hashes using a trusted independent SHA-256 implementation, validate `EVIDENCE_RECORD.json` against the exact referenced schema version, and review the limitations. Do not treat a process-exit match as gate closure.

The C recorder does not yet provide a general schema-validation command. Schema validation is a separate check and must itself be recorded when required.

## Non-installation boundary

This package is a side-by-side Alpha 1 candidate. Do not overwrite `tools/k501-run.sh`, install into the active workspace, rewrite old evidence, or publish the package based only on the package-local tests. Those actions require a separate compatibility, provenance, rights, and operator gate.
