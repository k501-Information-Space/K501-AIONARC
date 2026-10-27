# K501 AIONARC Pipeline v3 — AI Memory Runtime C Alpha 1

**Version:** `3.0.0-alpha.1`  
**Time anchor supplied for this work:** Unix `1792121537` — 2026-10-16 03:32:17 UTC / 05:32:17 CEST  
**Status:** NON-CANONICAL ENGINEERING CANDIDATE — controlled operator testing pending  
**QH256:** v2.0 core unchanged; mapping profile `QH256-K501-AIONARC-MEMORY-V3A1`  
**License split:** `LICENSES/FILE_LICENSE_MAP.md` (code MIT; authored documentation/conduct CC BY 4.0; excluded/upstream material keeps its own terms)

## Purpose

Alpha 1 is a compact C11 structural-memory runtime. It observes a declared filesystem tree read-only, creates append-only frames, retains source-bound text from supported text formats, uses QH256 v2 for a versioned application-level state mapping, and exposes verification, query, trace, history and diff operations. It is not a semantic reasoning engine.

## The controlling contract

Read `docs/MEMORY_CONTRACT_V1.0.md` first. It is normative for this Alpha 1 candidate and controls source behavior, mapping, text-status taxonomy, frame reconstruction, failure handling and acceptance tests. The complete 0–63 QH256 mapping table is in `docs/QH256_MAPPING_PROFILE.md`; cells 64–127 remain UNKNOWN/reserved. QH256 core algebra and its 32-byte wire representation remain unchanged.

Alpha 1 specifically corrects the Alpha 0 findings around invalid UTF-8 vs embedded NUL, PDF extractor outcome vs text acceptance, restoration of `text_status` before QH256 verification, final-index creation after candidate reconstruction, and empty-text TSV roundtrip. The package-local full regression suite now passes; the user's controlled unpack/test process and owner review remain pending. Package-local success is not a claim of target-host validation or canonization.

## Build and test

Requirements: Linux/POSIX, C11 compiler, Make and standard system headers. Qualified shipped binary: Linux x86-64 baseline; direct dynamic dependency `libc.so.6`; ELF interpreter `/lib64/ld-linux-x86-64.so.2`; required GLIBC symbol versions through `GLIBC_2.34`. This is not a universal Linux binary promise.

    ./scripts/build.sh
    make test
    ./scripts/verify-package.sh

The Makefile enforces `-march=x86-64 -mtune=generic` as final compiler flags even when `EXTRA_CFLAGS` is supplied. The existing source style still requires `-Wno-misleading-indentation`; this exception is disclosed in `docs/VALIDATION_REPORT.md`. A compiler is a build-time dependency, not required to run the prebuilt binary. PDF extraction optionally invokes `pdftotext`; its absence must not break structural scan, hashing, frame creation, verify or non-PDF text queries.

## Safe quickstart

Use a new, isolated, non-sensitive test directory and an empty store:

    mkdir -p "$HOME/K501_A1_TEST/source"
    printf 'proof before state\n' > "$HOME/K501_A1_TEST/source/README.md"
    build/k501-memory init "$HOME/K501_A1_TEST/store"
    build/k501-memory scan "$HOME/K501_A1_TEST/source" "$HOME/K501_A1_TEST/store" k501-a1-test frame-001 none
    build/k501-memory verify "$HOME/K501_A1_TEST/store" frame-001
    build/k501-memory query "$HOME/K501_A1_TEST/store" latest "proof before state"

Do not use canonical K501 source trees as the first test target. Source and store must not overlap. Existing frame IDs are never overwritten. See the operator manuals for failure handling.

## Data and claim boundaries

- `STATE.tsv` is structural source-observation state; it never stores model-generated interpretation.
- `TEXT.tsv` is derived extracted text, bound to source-object and text hashes.
- `INDEX.tsv` is derived; its validated status is required to deterministically reconstruct object QH256 values.
- `QH256.hex` is an aggregate state, not the archive or complete provenance history.
- A PASS means the declared local checks passed. It does not prove external truth, completeness, semantic understanding, protection from an adversary who can rewrite the entire store, or scientific superiority.
- No llama.cpp, embeddings, vector store, OCR, DOCX extraction or semantic result writer is included.

## License, provenance and transition

The old v2.1-C archive is not bundled in Alpha 1. Its earlier known SHA-256 is retained only in `docs/SOURCE_PROVENANCE.md`. The new prospective K501 license framework is in `LICENSES/FILE_LICENSE_MAP.md`; `LICENSES/LICENSE_TRANSITION_NOTICE.md` explains the owner's request to supersede earlier internal policy notices for future packages without purporting to revoke old public grants or third-party terms. This package has not been pushed to any public repository and no release has been published.

## Status and next action

Package-local build and full regression tests pass in the packaging environment. The next planned project step remains the user's controlled unpack and repeated test series via `k501-run.sh`, preserving Alpha 0 and all old evidence. G5-A must remain OPEN until the resulting evidence is reviewed. G5-C needs rights-holder scope review; G7 reproducibility remains a separate open gate.
