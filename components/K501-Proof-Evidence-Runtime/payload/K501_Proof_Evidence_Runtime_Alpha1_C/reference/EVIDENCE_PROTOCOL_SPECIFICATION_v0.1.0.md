# K501 Evidence Protocol — Technical Specification v0.1.0

## 1. Status and terms

**Status:** proposed specification awaiting operator review and explicit freeze approval.  
**Normative words:** **MUST**, **MUST NOT**, **SHOULD**, **SHOULD NOT**, and **MAY** express requirements for this protocol.  
**Evidence boundary:** no project gate was executed while preparing this proposal. Existing historical run folders are source material, not freshly revalidated results.

### 1.1 Evidence labels

Every statement in a gate report MUST be classifiable as one of:

- **FACT / OBSERVED:** directly present in an identified artifact or captured command output.
- **TEST RESULT:** observed result from a named test with captured input, environment and expected behavior.
- **DERIVED:** calculated from identified facts by a documented method.
- **SPECIFICATION:** an intended rule, contract or acceptance criterion.
- **ASSUMPTION:** a working premise not yet proven.
- **OPEN:** missing evidence, unresolved discrepancy, unknown mapping or uncompleted test.

A proposal or assumption MUST NOT be written as a verified fact. Historical PASS reports MUST remain historically scoped and MUST NOT be described as fresh tests.

## 2. Evidence chain

The required trace is:

`Claim / requirement → scope → exact subject identity → preconditions → command or procedure → raw capture → result evaluation → gate disposition → human-readable summary`.

Every link MUST be explicit. A reader or machine should be able to traverse from a claim to its test output and back to the exact tested artifact without relying on memory or inference.

### 2.1 Minimum evidence-record fields

The normalized evidence record MUST capture:

- schema ID and version;
- unique `evidence_id` and `run_id`;
- area and declared scope;
- gate/check ID and predeclared acceptance criteria;
- exact subject artifacts: name, version if known, byte size and SHA-256; for Git-backed subjects, repository identity, commit ID and working-tree status;
- command as an argv array or a complete documented procedure, working directory, relevant environment and expected exit behavior;
- start/end times in ISO 8601 UTC and Unix epoch seconds when available;
- actual execution result, exit code and expected exit code (nullable when not applicable);
- references and hashes for raw output/logs and derived reports;
- result status, limitations, reviewer and links to previous/retry/superseding records when applicable.

A field MUST be `null`, `unknown`, or explicitly absent only where its schema permits that state. Do not fill a missing fact with a guessed value.

### 2.2 Exact artifact identity

For a file or archive, the minimum identity is SHA-256 plus byte size. A basename alone is insufficient. For a Git subject, record at least the remote/local repository identity as applicable, commit hash, branch/tag when relevant, and `git status --short --branch` output or a digest of it. A clean working tree MUST NOT be inferred from a commit hash alone.

If an archive and an extracted directory are both tested, record identities for both. A successful checksum of an archive does not prove that an extracted copy or a repacked archive is identical to it.

## 3. Run, result and gate are distinct

- **Run:** a single recorded execution attempt with a unique run ID.
- **Evidence record:** the normalized machine-readable description of the run and its outputs.
- **Check result:** the evaluated outcome of one declared check.
- **Gate status:** the disposition of a gate after all required checks and acceptance criteria are assessed.
- **Snapshot:** a bounded capture of selected objects and observations at a documented capture boundary.
- **State Anchor:** a stable reference to a defined information state and scope. It can point to a snapshot and evidence, but a timestamp alone does not prove atomic capture or completeness.

A directory named like a run, a `FRAME.json` with `status_at_creation=RUNNING`, a command that completed, or an exit code of zero does not alone prove that a gate passed. Initial run metadata and final closeout status must remain distinguishable.

## 4. Status vocabulary

### 4.1 Check outcome

- `PASS`: the predeclared check criterion was met and the required evidence exists.
- `FAIL`: the check ran and the criterion was not met.
- `BLOCKED`: the check could not be run or interpreted because of a missing prerequisite, tool, input, permission or unresolved identity.
- `NOT_RUN`: no execution result exists.
- `PARTIAL`: some declared steps completed, but the full check criterion was not evaluated.

### 4.2 Gate disposition

- `OPEN`: evidence or acceptance criteria remain outstanding.
- `PARTIAL`: some required checks are complete; the gate is not closed.
- `PASS`: every mandatory acceptance criterion for the stated scope has sufficient evidence, with no unresolved blocker within that scope.
- `FAIL`: a mandatory criterion failed.
- `BLOCKED`: gate evaluation cannot proceed due to a prerequisite.
- `DEFERRED`: an explicit scope decision postpones the gate; the reason and decision reference are required.

`PASS` MUST NOT be used merely because a tool returned exit code `0`. `DEFERRED` MUST NOT be represented as `PASS`. A documented scope exclusion is not a claim that the excluded issue is solved.

## 5. Raw, derived and closeout records

1. **Raw capture:** exact command/procedure, stdout/stderr or recorder transcript, exit status and captured inputs. Preserve original bytes where possible.
2. **Derived evidence:** parsed inventories, JSON summaries, calculated digests, comparison tables and charts. Every derived item points to its raw input(s) and method/version.
3. **Closeout:** a human-readable decision identifying the gate, scope, required criteria, evidence references, blockers, reviewer, decision time and final disposition.

Derived reports MUST NOT replace raw evidence. The process MUST retain failed attempts and retries. Correcting an error creates a new run; the prior result stays intact and linked.

## 6. Run recorder contract

The established recorder is `~/K501_Information_Space/tools/k501-run.sh`; its outputs are stored under `~/K501_Information_Space/runs/<area>/` according to the operator's current workflow. The current known areas are `system`, `pipeline`, `eArc`, `QH256`, and `AIONARC_CHANNEL`.

Before this proposal is frozen, the actual recorder MUST be inspected and mapped to the logical requirements below. This proposal does not presume filenames or command-line flags that were not directly inspected.

A conforming capture MUST make available, directly or by an unambiguous mapping:

- unique run directory / run ID;
- start metadata, including command and working directory;
- exact subject identity and relevant environment;
- full output transcript or separate stdout/stderr;
- actual exit status and whether non-zero was expected;
- output-file inventory with byte sizes and SHA-256 digests;
- final status/closeout distinct from start metadata;
- links to earlier/retry/superseding runs;
- no silent overwrite of earlier evidence.

If the recorder currently stores a subset in other filenames, a mapping table MUST identify the actual filename/field. Missing logical fields remain `OPEN`; do not fabricate them.

## 7. Directory and naming convention

Retain the established path pattern where practical:

`runs/<area>/<unix-epoch>_<UTC timestamp>_<short-slug>/`

Run IDs SHOULD use UTC and be unique. Slugs SHOULD be concise, lowercase and descriptive; avoid personal information and secrets. Evidence must not be placed into an older run directory merely to make it appear part of that run.

A logical run directory should expose these content roles, whether in separate files or mapped fields:

- start metadata;
- command/procedure;
- raw output;
- exit result;
- artifact/input identity;
- output manifest;
- closeout/result;
- optional derived inventory/report.

Do not infer that a run has no files from a shallow `tree -L` view. A depth-limited tree is a structural view only; use `find` or a complete inventory for file-level assertions.

## 8. Standard command policy

Routine verification SHOULD use short, named, reusable check profiles composed of established terminal tools such as `sha256sum`, `unzip`, `tar`, `jq`, `find`, `sort`, `git`, `file` and `tree`. The operator SHOULD NOT have to repeatedly compose or paste large bespoke shell programs for standard checks.

- Check commands are kept in `CHECK_CATALOG.md` and referenced by stable check IDs.
- Official gate evidence MUST be run through the validated recorder invocation once its interface has been inspected and documented.
- Commands that write output MUST write only to the recorder/run output area or a declared disposable workspace—not to canonical package/source trees.
- No package install, code edit, checkout, clean/reset, deletion, rename, upload or repository mutation is permitted during read-only preflight without a specific GO.
- If an optional utility is missing, report `BLOCKED` or `NOT_RUN` as applicable. Do not silently install it or replace the test with an unapproved script.

## 9. Gate closure requirements

A gate closeout MUST state:

1. gate ID and version of its criteria;
2. exact scope and explicit exclusions;
3. subject identities and SHA-256 values;
4. required checks and their IDs;
5. each check's expected and observed result;
6. links to raw and derived evidence;
7. deviations, failures, retries and unresolved points;
8. conclusion plus its limits;
9. reviewer and UTC decision time;
10. gate status using the vocabulary above.

For `PASS`, all mandatory criteria must be met. A known issue outside the declared scope may be non-blocking only when the exclusion is explicit and approved. A known issue inside scope blocks PASS until dispositioned.

## 10. Human-readable output standard

Every check report SHOULD follow this order:

1. **Purpose** — one sentence.
2. **Scope / subject** — exact package or commit, version and digest.
3. **Preconditions** — required files, tools, paths and environment.
4. **Command/procedure** — exact, reproducible invocation.
5. **Expected result** — defined before execution.
6. **Observed result** — factual and linked to captured output.
7. **Disposition** — `PASS`, `FAIL`, `BLOCKED`, `NOT_RUN` or `PARTIAL`.
8. **Limitations / open points** — clear, short and not hidden in prose.
9. **Evidence links** — run ID, output filenames and checksums.

Machine-readable JSON and human-readable Markdown MUST be generated from or reconciled against the same record. The summary MUST not contain a stronger claim than the evidence.

## 11. Mapping and navigation requirements relevant to the publication

The QH256 state algebra, the implemented mapping, Gateway, Index, Namespace and navigation implementation are distinct but related claims. The publication MUST identify each claim's specific artifact and validation method.

The operator's current statement that 63 QH256 cells are used and the remainder are `UNKNOWN` is a **mapping assertion to verify against the canonical mapping profile and implementation**, not a fact established by this protocol package. The verification must establish cell numbering (zero-based or one-based), exact field-to-cell assignments, the count of mapped cells, the state of all remaining cells, and consistency between specification, implementation and test vectors. Historical conflicting mapping counts MUST be preserved as historical statements and reconciled explicitly, not silently edited away.

Gateway/Index existence does not prove referential integrity. The navigation package must test target existence, namespace classification, reference resolution and behavior for unresolved targets. Each result is scoped to the exact archive/commit tested.

## 12. Privacy and publication boundary

The full Conversation PDF is contextual source material and is not part of the initial public package unless a distinct privacy and rights review explicitly authorizes it. The State Anchor and a deliberately bounded, privacy-reviewed snapshot are the intended operational example. Public metadata, file contents, path names, logs and embedded document metadata all require review.

## 13. Change control

After freeze, this specification is immutable within its version. Changes require a new version or a separately approved amendment, a change record, updated checksums and a new review. Existing evidence records and historical gate outcomes are not rewritten to conform to a later schema. Legacy evidence may be mapped by a documented adapter/interpretation record; the original records remain unchanged.
