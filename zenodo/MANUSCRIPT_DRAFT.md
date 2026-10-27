# K501 AIONARC Integrated Pipeline & Proof Runtime v3.5.0

**Scientific/software publication draft — not approved for upload**  
**Version:** 3.5.0 aggregate integration candidate  
**Author attribution:** Patrick Robert Miller (alias `IInkognit0`), per direct owner declaration recorded at Unix `1793046600`.  
**Publication date, license, access right and DOI:** not assigned.

## Abstract

This software package integrates a local structural-memory pipeline with a controlled process-evidence recorder. The pipeline observes a declared filesystem source, creates append-only frames, binds extracted text and indexes to observed objects, and applies a versioned application mapping over the unchanged QH256 v2 state algebra. The recorder captures explicit command invocations, bounded host/run metadata, merged process output, observed exit status and SHA-256 manifests. Integration is exercised as a separate child-process call, preserving the components' operational boundaries. The package provides a common build and validation entry point and records source and executable identities. The evidence model distinguishes run execution from gate acceptance; checksums establish byte consistency but do not authenticate authorship or prove semantic truth. The owner designated this local aggregate canonical and ratified MIT for K501-authored code and CC BY 4.0 for K501-authored documentation. Component versions and third-party notices remain visible; GitHub and Zenodo publication are separate and have not occurred.

## 1. Motivation and contribution

Data ingestion pipelines often collapse observed files, derived text, indexes, compact state summaries, command output and conclusions into one undifferentiated result. K501 separates these roles. AIONARC supplies structural observations and a bounded retrieval path. The Proof/Evidence Runtime supplies process-level traceability for an explicitly invoked command. This aggregate contributes a single local source bundle, top-level build/test orchestration, and an integration check without claiming that one component validates the semantic correctness of the other.

## 2. System design

The pipeline operates over a declared source root and a separate output store. Its frame and record formats preserve object identity, source digests, temporal transitions, derived text and validation evidence. The V3A1 application mapping assigns documented observations to QH256 cells while leaving reserved cells unassigned. QH256 algebra and serialization are not changed.

The evidence recorder accepts an explicit argument vector, creates a unique run directory, captures command and host metadata, executes the child as a separate process, retains a transcript and result, and hashes the evidence files. Its exit criterion is a process-level observation. Its record explicitly has no gate effect until reviewed. No status-to-QH256 mapping is defined.

## 3. Validation method

The aggregate validation profile is composed from the two package test suites, a clean build and a bounded recorder-to-runtime process integration test. Each test must be bound to the exact source tree, built executable and environment record. Structural checks verify JSON records, package manifests, aggregate schemas, Index, manifest and checksums. The validation report must distinguish fresh run evidence from historical package reports.

## 4. Results

The controlled local aggregate build-and-test run `1793045778_20261026T201618Z_k501-v350-integrated-build-and-test` completed successfully on Debian 13 x86-64, using GCC 14.2.0-19 and GNU Make 4.4.1. The fresh AIONARC build passed QH256 core, SHA-256, V3A1 mapping, and its full declared regression suite. The Proof/Evidence Runtime suite passed 60 assertions with zero failures while invoking the exact AIONARC executable built from this aggregate source tree. The recorder self-test passed. Resulting executable SHA-256 values are `838746254724916d60c53fd1de5cf112938271b991e127e9a29bab547d019ff2` (AIONARC) and `e9a9b46079097b9b72769cd0119218acb76f9ca7faef7d330c0a02f7651ca880` (Proof/Evidence Runtime).

Source archive sidecars and byte-for-byte payload parity were checked in `1793045823_20261026T201703Z_k501-v350-source-archive-binding`: 52 files for each AIONARC ZIP/TAR.GZ and 21 files for the Proof/Evidence ZIP. Aggregate schema and registry checks parsed 41 JSON files. The final root Index, manifest, checksum sidecar, and integrated-artifact verification are recorded in the included validation report. These results describe bounded local technical evidence, not a formal proof or public release. The owner-ratified rights scope is recorded in the aggregate file map; third-party material remains under its preserved terms. Independent build and host gates are documented separately. No aggregate ZIP digest is asserted in this manuscript; the review archive has an adjacent SHA-256 sidecar. Publication metadata remains draft because no Zenodo deposit has been created.

## 5. Limitations and threat model

The tested workflow is local and single-writer. SHA-256 manifests are not signed attestations or independent trust anchors. The recorder merges stdout and stderr, captures only bounded host/environment details, and cannot defend against an actor able to rewrite the whole workspace. AIONARC retrieval is lexical, not semantic; PDF extraction is optional and delegated to an external tool. No LLM, embeddings, OCR, encryption, trusted timestamp authority, remote witness, general concurrent-writer protocol, or hostile-store guarantee is provided. A successful bounded suite does not prove external truth, all possible state behavior, universal portability, or scientific superiority.

## 6. Reproducibility and availability

The final publication must contain the approved source artifact, component provenance and license files, build instructions, test/verification instructions, and a checked archive checksum. The exact commit/tag, archive hash, compiler version, flags, operating system, architecture and GLIBC boundary should be recorded. A reproducible-build claim requires a separately specified second clean build and byte comparison; it must not be inferred from one build or from a matching source archive hash.

## 7. Rights and publication status

The owner ratified mapped K501-authored files under MIT (software/code) and CC BY 4.0 (documentation/descriptive material). Existing upstream notices remain applicable. Zenodo's deposit-level access/license fields remain unset pending a separate publication choice. This manuscript is a draft and no Zenodo deposition or DOI has been created.

## 8. References and citation

Use the versioned citations and provenance included in each component. Add only references that have been checked for relevance and bibliographic accuracy during owner review. Do not imply that a DOI or public release exists before publication.
