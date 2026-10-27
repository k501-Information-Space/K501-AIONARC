# Build and Validation Report — K501 Proof and Evidence Runtime Alpha 1 (C11)

**Package identity:** `K501_Proof_Evidence_Runtime_Alpha1_C`  
**Version:** `0.1.0-alpha.1`  
**Disposition:** build and bounded tests PASS; package remains a non-canonical engineering candidate.  
**User request / GO anchor:** `1792437767` — 2026-10-19 19:22:47 UTC / 21:22:47 CEST.  
**Build/report observation anchor:** `1792438732` — `2026-10-19T19:38:52Z`.

## 1. Compiled artifact

- Executable: `bin/k501-proof-evidence-linux-x86_64`
- SHA-256: `e9a9b46079097b9b72769cd0119218acb76f9ca7faef7d330c0a02f7651ca880`
- Size: `49656` bytes
- Format: ELF 64-bit LSB PIE, x86-64, dynamically linked for GNU/Linux.
- Language: C11 / POSIX process and file interfaces.
- Build flags: `-O2 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror -std=c11`.
- Toolchain observed in this build environment: GCC `14.2.0-19`; Linux x86_64; glibc `2.41-12+deb13u3`.

The user's operator host was not directly accessed or changed. Host-level installation and validation remain unperformed.

## 2. Test results

| Check | Result | Scope |
|---|---|---|
| Strict GCC C11 compile | PASS | Recorder, QH256 module, SHA-256 module; warnings treated as errors |
| Native C test harness + existing Runtime Alpha 1 integration | PASS, 60 assertions, 0 failures | Success path, declared expected exit 23, exit mismatch, evidence files, manifest checks and child invocation of the existing runtime's `qh256-test` |
| C self-test | PASS | SHA-256 vectors and QH256 v2 state/cell/MERGE_K/serialization checks |
| JSON Schema validation | PASS, 4 records | Each generated `EVIDENCE_RECORD.json` validated against bundled draft schema v0.1.0 |
| Run manifest and raw-reference hashes | PASS | All 4 test runs; 9 files per run and referenced raw evidence hashes/sizes checked |
| GCC AddressSanitizer + UndefinedBehaviorSanitizer build/test | PASS, 60 assertions, 0 failures | Separate instrumented build and Runtime Alpha 1 integration test |
| Repeated clean build byte comparison | PASS | Two strict GCC builds produced identical executable SHA-256 |

The deliberately mismatched exit test produced a `FAIL` evidence status as expected; the test harness itself passed. The declared expected exit code 23 test produced a `PASS` process-criterion result, not a project gate result.

## 3. Exact external test subject

The integration run invoked the existing Runtime Alpha 1 binary as a separate process. It was not linked into or copied as part of the Proof/Evidence executable.

- Alpha 1 runtime archive: SHA-256 `f14e203ad7bfa466d118780b06b69e59198e2fd917cb0e7e82222b83557df607`, `151501` bytes.
- Extracted child executable: SHA-256 `838746254724916d60c53fd1de5cf112938271b991e127e9a29bab547d019ff2`.
- Observed child test: `qh256-test` returned exit 0 and emitted `QH256 CORE TEST PASS`.

This demonstrates one bounded process-level integration path. It does not prove all Runtime modules, mappings, corpus operations or gates.

## 4. Reused QH256 and SHA-256 source identity

The following source files were byte-compared with the corresponding files extracted from the tested Alpha 1 runtime archive. The comparison passed; their content was not intentionally modified in this package.

- `src/qh256.c`: `5414029d98fafe098cb768f2a4f4aca2e8026fe66b50f5e6c10dc0f2fb1fcf3e`
- `include/qh256.h`: `786d7c0dfb58d583d5e10bb55593f382f5355c1af4a80f39da2b576e9e5fa76c`
- `src/sha256.c`: `42d4d57d09cdbab2b77110ee2752d8efa1c6f271fbbf37dc6ce01e599ffcbf12`
- `include/sha256.h`: `0ec9768ecf3c27939067fdaffee84507c30914e2adfed12ead260ee537c4546f`

The Evidence Runtime does not map Evidence statuses into QH256 cells. No new mapping is defined here.

## 5. Protocol and conformance boundaries

- The Evidence Protocol v0.1.0 and JSON schema are reference drafts; they remain proposed and not ratified.
- Protocol specification SHA-256: `3d5d39c7896e3a673ff8a5a3cb3c6acf52d55e4ef1ec72dc9cd4dbcebe603b7f`.
- Evidence schema SHA-256: `5948e2a839cbd5a1e687b950aeece3bd5d199ec69a3095fbcc51d46dd72a19f6`.
- The C recorder reuses the current run namespace and protocol identifier, but is not byte-compatible with `tools/k501-run.sh`; it is not an authorized in-place replacement.
- Existing user workspace files, prior run folders, Git repositories, GitHub state, Zenodo records and canonical project state were not modified by this build.
- License/public redistribution clearance remains OPEN; see `LICENSE_STATUS.md`.

## 6. Known limitations

- This package does not compile the entire abstract/historical K501 Information Space into one binary. It compiles one bounded operational component within that space.
- Standard output and standard error are merged in `TRANSCRIPT.log`.
- Environment capture is bounded and is not a full process-environment snapshot.
- SHA-256 manifest checks are not signatures or independent trust anchors.
- No gate is accepted by a matching exit code; every record declares `NO_GATE_EFFECT_UNTIL_REVIEW`.
- No injected disk-full or transcript-write-failure test was performed; that failure path is handled in code as `PARTIAL` when the exit criterion matches and the tool returns recorder error 73.
- Target-host validation, protocol ratification, compatibility approval, mapping review, license review and release acceptance remain OPEN.

## 7. Conclusion

**Compiled C11 candidate: PASS. Bounded integration/evidence tests: PASS. Canonical release acceptance: NOT CLAIMED.**

This report records the scope of checks performed in the current build environment. It does not authorize installation, replacement, publication or closure of any existing K501 gate.
