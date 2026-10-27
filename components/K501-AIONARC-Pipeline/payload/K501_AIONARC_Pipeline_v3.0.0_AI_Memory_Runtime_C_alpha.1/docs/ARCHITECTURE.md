# K501 AIONARC Pipeline v3 Alpha 1 — Architecture

Alpha 1 is a small terminal-first local structural-memory runtime written in C11. It observes a declared filesystem tree read-only, assigns stable per-root/path/type identities, creates append-only temporal frames, captures supported text as a derived layer, maps object and frame evidence to QH256 v2, and exposes verification, trace, history, diff and exact lexical query commands.

## Layers and boundaries

1. **Observed source:** filesystem nodes and byte streams under an operator-declared root. Never intentionally mutated.
2. **Structural frame:** versioned state records, stable object identity, record hash, content digest, times and type. No extracted prose and no LLM output.
3. **Derived text:** only captured candidates; UTF-8 and NUL rules are explicit; optional `pdftotext` is a subprocess adapter, not a core dependency.
4. **Derived index:** validates object/text association, path roundtrip, status taxonomy and the application-level QH256 value. In verify, statuses are reconstructed from the index before QH recomputation.
5. **QH256 state:** the unchanged QH256 v2 128-cell/32-byte core; Alpha 1 mapping profile V3A1 assigns cells 0–63 and leaves 64–127 UNKNOWN/reserved.
6. **Proof/manifest:** local contract outcome and byte-digest binding. Integrity is not truth.
7. **Retrieval:** exact lexical AND; result to verified frame and source record trace; no semantic inference.

## Version boundaries

Alpha 0 is a historical, immutable candidate. Alpha 1 uses distinct `V3A1` store/frame/text/index/proof schema IDs and `QH256-K501-AIONARC-MEMORY-V3A1`; an Alpha 0 store must be refused by Alpha 1. The QH256 v2 algebra and serialization do not change. Object-ID and object-record formulas are retained deliberately; the frame-chain domain and schema namespace are versioned to A1.

## Build/runtime boundary

The qualified target is Linux x86-64 baseline, requiring direct `libc.so.6`, a compatible ELF loader and glibc symbols through `GLIBC_2.34` for the shipped binary. Build flags specify `-march=x86-64 -mtune=generic`; a compiler is needed to build from source, not to run an installed binary. Optional `pdftotext` is not required for structural scan, hashing or verification. The local user workstation and reproducible build remain to be tested under the controlled operator workflow.

## Explicit non-goals

No llama.cpp integration, semantic inference, embeddings, vector store, OCR, document conversion pipeline, model admission, multi-writer/locking, remote trust anchor, signed proof, encryption, or hostile-store protection is included. DOCX remains structural-only. No source prose is promoted automatically to canonical truth.
