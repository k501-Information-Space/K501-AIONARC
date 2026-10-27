# K501 v3.5.0 Integrated Artifact — Validation Report

**Artifact:** K501 AIONARC Integrated Pipeline & Proof Runtime, `3.5.0`  
**Local state:** owner-declared canonical; full local gate profile passes  
**Owner Zeitanker:** Unix `1793046600` — 2026-10-26 20:30:00 UTC / 21:30:00 Europe/Berlin  
**Public actions:** none; no GitHub commit, push, tag, release, Zenodo upload, or DOI

## Decision

The owner directly ratified the aggregate as canonical locally and declared MIT for K501-authored software/code and CC BY 4.0 for K501-authored documentation/descriptive material. The rights map classifies every regular file in the distributable package; 296 paths are mapped and there are no unresolved paths. Third-party and QH256 notices and terms remain scoped separately. The declarant's identity and chain of title were not independently verified.

All local gates are PASS for the source hashes, binaries, host, and criteria described here. `OWNER_DECLARED_LOCAL_CANONICAL` evaluates PASS. GitHub and Zenodo publication gates remain OPEN: no aggregate repository target was declared, both previously proposed component targets returned 404 through the connected GitHub API, and no publication/deposit exists. Thus local canonicality is established by the owner's declaration; public GitHub canonicality and Zenodo publication are not established.

## Source identity

Controlled source audit `1793045823_20261026T201703Z_k501-v350-source-archive-binding` verified archive sidecars and archive-to-payload parity, rejected unsafe or duplicate paths, and confirmed the preserved component wrapper trees match their work trees outside generated build directories.

| Source archive | SHA-256 | Parity |
|---|---|---|
| AIONARC ZIP `K501_AIONARC_Pipeline_v3.0.0_AI_Memory_Runtime_C_3.0.0-alpha.1.zip` | `f14e203ad7bfa466d118780b06b69e59198e2fd917cb0e7e82222b83557df607` | PASS, 52 files |
| AIONARC TAR.GZ `K501_AIONARC_Pipeline_v3.0.0_AI_Memory_Runtime_C_3.0.0-alpha.1.tar.gz` | `0e124a4786e07dd84936f2ec1d4246a56ed0a057711766d8ced78ab3724e5821` | PASS, 52 files |
| Proof/Evidence ZIP `K501_Proof_Evidence_Runtime_Alpha1_C_0.1.0-alpha.1.zip` | `c48fd8863b3b5b8889f46af1efbc768281490b60ab4433700b1331793dfcf435` | PASS, 21 files |

The source import run is `1793045289_20261026T200809Z_k501-v350-integrated-source-import`. Embedded component trees and historical component license/gate records remain byte-preserved; the owner ratification applies to the integrated v3.5.0 aggregate path map only. Earlier audits remain historical records and are superseded only within this aggregate's current scope.

## Build, regression and integration

Controlled aggregate build/test `1793045778_20261026T201618Z_k501-v350-integrated-build-and-test` passed. The post-ratification final `make verify` run is `1793047970_20261026T205250Z_k501-v350-release-candidate-final-verify`; evidence and transcripts are under `evidence/runs/`.

| Check | Result |
|---|---|
| Clean strict C11 build of both executables | PASS |
| AIONARC QH256 core, SHA-256 vectors, V3A1 mapping and full regression suite | PASS |
| Proof/Evidence integration suite with exact same-tree AIONARC child | PASS, 60 assertions, 0 failures |
| Proof/Evidence self-test | PASS |
| Aggregate schemas and contract checks | PASS |
| Component package checksums and aggregate integrity verification | PASS |

The tests cover core and mapping behavior, frame transitions, proof-before-state, index/status reconstruction, empty-text round trip, UTF-8/NUL handling, file limits, PDF adapter paths, exact lexical retrieval, trace/history/diff, source parity/drift and tamper/refusal cases. This finite test suite is not an exhaustive formal proof.

## Host and reproducibility

Host acceptance run `1793047801_20261026T205001Z_k501-v350-host-acceptance` passed on Debian 13 Linux x86-64. It exercised the documented init/scan/verify/query flow, confirmed rejection of source/store overlap, and confirmed recorder child capture and evidence-record generation. It applies to this host and these exact binary hashes, not all Linux systems.

Same-host reproducibility run `1793047805_20261026T205005Z_k501-v350-reproducibility` built two independent clean copies of each pinned component source tree using the same host and toolchain. For both components, copy A, copy B, and the aggregate executable hashes matched. This demonstrates same-host clean-copy reproducibility, not cross-platform or independent-toolchain reproducibility.

| Executable | SHA-256 | Observed ELF boundary |
|---|---|---|
| `dist/linux-x86_64/k501-memory` | `838746254724916d60c53fd1de5cf112938271b991e127e9a29bab547d019ff2` | x86-64 PIE; maximum referenced GLIBC symbol `GLIBC_2.34` |
| `dist/linux-x86_64/k501-proof-evidence` | `e9a9b46079097b9b72769cd0119218acb76f9ca7faef7d330c0a02f7651ca880` | x86-64 PIE |

## Dynamic gate disposition

| Gate | State | Basis |
|---|---|---|
| `SRC-01` | PASS | Archive identities, sidecars, source parity and component comparison |
| `BLD-01` | PASS | Clean aggregate build and executable hashes |
| `TST-01` | PASS | Component suites and self-tests |
| `INT-01` | PASS | 60/60 integration assertions against the same-tree executable |
| `SCH-01` | PASS | Schema, specification, manifest, index and checksum validation |
| `RIGHTS-01` | PASS | Owner ratification and complete 296-path license map |
| `HOST-01` | PASS | Local Linux x86-64 operator quickstart and negative test |
| `REPRO-01` | PASS | Two clean copies per component match each other and aggregate binaries |
| `OWNER-01` | PASS | Direct owner declaration ratifies this local aggregate as canonical |
| `GITHUB-01` | OPEN | No exact aggregate destination, published ref, release or reconciled asset |
| `ZENODO-01` | OPEN | Draft only; no deposit or DOI |

## Limits and evidence locations

Hashes establish byte identity, not semantic truth, correctness for all possible states, legal ownership, or security against a hostile workspace. No LeakSanitizer pass is claimed; the earlier audit records ASan/UBSan runs with LeakSanitizer disabled due to the host's ptrace limitation. No other-host acceptance is claimed. Zenodo's deposit-level license/access fields remain unset because this package has mixed file-level scope; the draft is not approved or uploaded.

- `evidence/runs/1793045289_20261026T200809Z_k501-v350-integrated-source-import/`
- `evidence/runs/1793045778_20261026T201618Z_k501-v350-integrated-build-and-test/`
- `evidence/runs/1793045823_20261026T201703Z_k501-v350-source-archive-binding/`
- `evidence/runs/1793047801_20261026T205001Z_k501-v350-host-acceptance/`
- `evidence/runs/1793047805_20261026T205005Z_k501-v350-reproducibility/`
- `evidence/runs/1793047970_20261026T205250Z_k501-v350-release-candidate-final-verify/`
- `validation/GITHUB_TARGET_CHECK.md`
- `zenodo/METADATA_DRAFT.json` and `zenodo/MANUSCRIPT_DRAFT.md`
