# Alpha 1 Status Register

**Package version:** `3.0.0-alpha.1`  
**Status:** engineering candidate; NON-CANONICAL; not release-approved.  
**Time anchor supplied for the work:** `1792121537`.

| Item | Alpha 1 candidate state | Required evidence / limitation |
|---|---|---|
| QH256 v2 core | Algebra/serialization unchanged; mapping application versioned as V3A1 | `qh256-test`, mapping tests and full suite |
| Mapping cells 21/22/23/25 | Corrected in source/profile | Container unit/end-to-end suite now passes; G5-A remains OPEN until the controlled operator-side series and exact package audit |
| Invalid UTF-8 / embedded NUL | Separate statuses | Both frames scan and verify in the package-local regression suite |
| Empty text capture | Explicit `CAPTURED_EMPTY`; empty TSV field roundtrip fixed | End-to-end scan/verify test passes |
| Per-object cells 27–30 | Final index generated after candidate/path reconstruction checks | Full proof-order audit and test transcript |
| Optional PDF | Extractor outcome separate from text acceptance | Package-local tests with and without `pdftotext` pass; operator host rerun remains |
| Memory contract | `docs/MEMORY_CONTRACT_V1.0.md` normative for A1 | This package, gate results and owner review are needed before canonization |
| Platform ABI | Qualified scope is Linux x86-64 baseline/glibc symbols through 2.34 | Container ELF audit is not the user's target-host test |
| License scheme | MIT for code; CC BY 4.0 for authored documentation/conduct; source rights map | Rights-holder's authority is a declared prerequisite; no legal opinion is asserted |
| Historic v2.1-C archive | Not redistributed inside A1 | Prior hash recorded in `SOURCE_PROVENANCE.md` |
| Reproducible build | OPEN / NOT CLAIMED | Gate G7 remains separate |
| Compiler warning policy | `-Wno-misleading-indentation` retained explicitly | Source readability cleanup is not claimed complete |
| Python/C v2.1 parity | OPEN / NOT CLAIMED | Not changed by A1 |
| Model integration | Absent by design | Future separate admission/security contract required |
| Public repository/release | Not changed or published by this operation | Explicit later approval needed |

## Overall gate interpretation

Alpha 1 includes fixes and new tests for defects found during Alpha 0 validation. The package-building process in this workspace is an engineering pass only. The next state is **controlled unpack and operator-side test series**, not `G5-A PASS`, `G5-C PASS`, `G7 PASS`, or canonical release.
