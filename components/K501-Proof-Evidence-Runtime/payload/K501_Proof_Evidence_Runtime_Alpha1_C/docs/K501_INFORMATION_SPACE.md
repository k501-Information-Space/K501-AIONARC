# K501 Information Space — Definition and C11 Realization

## 1. Definition

The **K501 Information Space** is the shared, reconstructable structure within which K501 identities, namespaces, contracts, objects, state representations, temporal frames, provenance, evidence, and relations are defined and connected.

It is not identical to any one executable. It is not an LLM, shell script, runtime binary, retrieval engine, or user interface. Those may implement or access particular operations, but none is the Information Space itself.

A native C compilation can implement the Information Space's bounded operational rules. It cannot compile all historical evidence, external repositories, published records, or abstract structure into one binary and thereby make them one artifact. Those retain their identities and provenance; the C program refers to them through defined relations.

## 2. One coherent space, distinct operations

The shared space has one controlling rule: every operation acts on identified inputs under an identified contract and produces an identifiable result. Operations remain distinct even when they participate in one information space.

```text
K501 INFORMATION SPACE
  ├── Namespace and contract layer
  ├── K501 AIONARC Runtime — observe source, create frames, reconstruct state
  ├── QH256 v2 core — represent and combine four-state cells
  └── Proof and Evidence Runtime Alpha 1 — capture execution and evidence
```

This diagram represents logical boundaries, not a claim that all these components were compiled into the same binary. In this package, the Proof/Evidence component is native C11 and the QH256 v2 module is linked as an independently defined source module. The existing AIONARC Runtime is invoked as a separate process when supplied by the operator.

## 3. State and proof are not collapsed

- **Observation** records what was encountered at a declared source boundary.
- **Runtime state operations** produce versioned structural records and frames according to the Runtime's contract.
- **QH256 operations** act on the established 128-cell, 2-bit state structure: `UNKNOWN=00`, `FALSE=01`, `TRUE=10`, `GUARD=11`. The v2 core remains unchanged.
- **Proof/Evidence operations** capture an invocation, its subject identity, environment summary, output transcript, actual exit code, expected behavior, and evidence-file hashes.
- **Gate assessment** determines whether all mandatory gate criteria are met. The recorder does not perform this decision. Its records explicitly state `NO_GATE_EFFECT_UNTIL_REVIEW`.

A successful process exit is a FACT about that process. It does not, by itself, prove the truth of its output, the completeness of a corpus, QH256 mapping correctness, or completion of a gate.

## 4. C11 as implementation language

The recorder, its evidence-record generator, its file-hashing path, and its QH256 self-test are compiled from C11 source. The only build description is a Makefile; there is no shell script acting as the recorder implementation. The Makefile uses the normal system build toolchain.

The recorder passes explicit argument vectors through `execvp`, rather than synthesizing a shell command string. This avoids giving shell parsing or interpretation an implicit role in execution. A caller can intentionally invoke a shell as the explicit child command, but the recorder itself does not insert one.

## 5. Namespace continuity

The run namespace reuses the established path shape:

`runs/<area>/<unix-epoch>_<UTC timestamp>_<slug>/`

The recorder retains the controlled-run protocol identifier `K501-CONTROLLED-RUN-0.1`, the familiar evidence filename roles, and the currently established area names. It adds `EVIDENCE_RECORD.json` to provide a normalized machine-readable evidence layer.

This is **logical namespace compatibility**, not byte-for-byte output compatibility with the Bash recorder. In particular, some human-readable fields and the `COMMAND.txt` representation differ. Therefore this candidate is not yet an authorized in-place replacement for `tools/k501-run.sh`. A separate comparison against the namespace contract and operator workflow is required before installation.

## 6. Information Space status

This package is a compiled implementation candidate inside the K501 Information Space. It does not establish the whole Information Space as fully compiled, fully integrated, canonical, or complete. Those are separate, scoped claims requiring their own evidence and explicit operator disposition.

**PROOF BEFORE STATE. NO DRIFT.**
