# Validation and Release Gates

The machine profile is `specifications/RELEASE_GATE_PROFILE_v3.5.json`. This local aggregate status model does not replace or rewrite the embedded upstream G0–G12, RT, G5, or XG records.

## Status rules

- `PASS`: the declared criterion has evidence for its exact scope.
- `FAIL`: a required criterion ran and failed.
- `OPEN`: evidence or required publication/review is absent.
- `PARTIAL`: only part of the declared gate was exercised or capture is incomplete.
- `SKIP`: the release profile excludes a capability and records why.

Required-gate readiness is conjunctive. A zero process exit never promotes a gate automatically. A source, binary, environment, test, license-scope or claim change invalidates affected evidence and requires recalculation. Preserve failed runs; retries use new run IDs.

## Gate order

1. `SRC-01`: bind source archives and copied component trees; check nested manifests and aggregate identity.
2. `BLD-01`: clean-build both components and record toolchain, flags and output hashes.
3. `TST-01`: run component self-tests and regression suites.
4. `INT-01`: run recorder integration against the exact AIONARC binary built at step 2.
5. `SCH-01`: validate schemas/specifications and aggregate indexes, manifest and checksums.
6. `RIGHTS-01`: record the owner's file-level license ratification and preserve third-party notices.
7. `HOST-01`: exercise the declared local Linux x86-64 operator path and negative overlap case.
8. `REPRO-01`: build two isolated copies of the pinned source trees on the same host/toolchain and compare outputs.
9. `OWNER-01`: record the owner's canonical designation for the exact local aggregate/version.
10. `GITHUB-01`: verify the exact repository target, publish commit/tag/assets and reconcile downloads.
11. `ZENODO-01`: approve final metadata and rights/access, deposit the reviewed asset, then reconcile DOI and checksum.

## Current release state

The owner declared aggregate version 3.5.0 canonical locally and ratified MIT for K501 software/code and CC BY 4.0 for K501 documentation/descriptive material at Unix epoch 1793046600. The path-complete license map covers the distributable tree. Existing upstream and QH256 notices remain in force. The owner identity and chain of title were not independently verified.

The local build, component tests, 60-assertion integration suite, schema/integrity checks, operator smoke run and same-host twin clean-copy builds passed for the bound source and binary hashes. This supports `OWNER_DECLARED_LOCAL_CANONICAL`; it does not establish public GitHub canonicality.

`GITHUB-01` remains open because the two proposed component repository lookups returned 404 and no aggregate destination is declared. No publication occurred. `ZENODO-01` remains open because the deposit is still a draft and no DOI exists. `GITHUB_PUBLICATION_PREFLIGHT` is ready for review; it is not a published release. The stable GitHub and Zenodo publication profiles remain blocked pending their own evidence.

## Stop conditions

Source/hash mismatch; failing test; recorder/build outputs not bound to the same candidate; schema/index/manifest drift; unsupported claim; unresolved upstream rights; target-host mismatch; non-reproducibility; wrong or unknown GitHub target; or mismatch between uploaded and downloaded assets.
