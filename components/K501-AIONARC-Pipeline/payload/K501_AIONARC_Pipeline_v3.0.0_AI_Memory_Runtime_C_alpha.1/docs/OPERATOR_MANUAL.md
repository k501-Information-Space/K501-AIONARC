# Operator Manual — Alpha 1 Runtime

**Audience:** human operator at a Linux terminal. This is a test candidate, not a production service.

## 1. Safety boundary

Always unpack the package into a new, isolated directory. Do not unpack over Alpha 0, canonical K501 repositories, real research source directories, or the frozen `/home/hirn/K501/` root. Start with generated fixtures and an empty store. Keep source and store outside each other. Preserve unexpected staging artifacts and full output.

## 2. Build

Requirements: Linux x86-64 or another compatible POSIX environment, `cc` supporting C11, Make, and standard libc/POSIX headers. The qualified release baseline is `-march=x86-64 -mtune=generic`; do not use a native/host-specific build for the release contract. PDF text extraction is optional and uses `pdftotext` when available.

    ./scripts/build.sh
    make test

`make test` creates disposable synthetic test data in a temporary directory and removes those test fixtures after the run. It does not scan the operator's canonical workspace.

## 3. Basic use with generated test data

    mkdir -p "$HOME/K501_A1_TEST/source"
    printf 'proof before state\n' > "$HOME/K501_A1_TEST/source/README.md"
    build/k501-memory init "$HOME/K501_A1_TEST/store"
    build/k501-memory scan "$HOME/K501_A1_TEST/source" "$HOME/K501_A1_TEST/store" k501-a1-test frame-001 none
    build/k501-memory verify "$HOME/K501_A1_TEST/store" frame-001
    build/k501-memory query "$HOME/K501_A1_TEST/store" latest "proof before state"
    build/k501-memory verify-source "$HOME/K501_A1_TEST/store" frame-001 "$HOME/K501_A1_TEST/source"

A successful scan includes `K501 MEMORY SCAN COMMIT PASS` and `proof_before_state=PASS`. Verification must independently print `VERIFY PASS`. If any command fails, stop and preserve output; do not manually edit `STATE.tsv`, `INDEX.tsv`, `QH256.hex`, or manifests.

## 4. PDF behavior

PDF extraction is optional. Without `pdftotext`, a PDF must remain in `STATE.tsv` and `INDEX.tsv` as `STRUCTURAL_ONLY` with status `PDF_EXTRACTOR_UNAVAILABLE_OR_FAILED`. Frame creation and verification should continue; other captured text remains searchable. When the extractor runs successfully, cell 23 records process success even if the output is later rejected or skipped. See the Memory Contract for statuses and QH mapping.

## 5. Frame identifiers and parents

Frame IDs are unique, append-only, and cannot be reused. Use `none` only with an empty store. For later frames, specify the exact parent frame ID or use `latest` only if there is a single unambiguous tip. When branches exist, the runtime refuses to guess.

## 6. Diagnostics

- `VERIFY FAIL`: stop; do not fix artifacts manually.
- `SOURCE DRIFT`: the current source no longer matches the selected snapshot; historical frame is still historical evidence.
- `OPEN: multiple frame tips`: specify the intended parent explicitly.
- `.stage-*`: interrupted/uncommitted evidence. Preserve and inspect; do not auto-promote or delete.

## 7. Limitations

This is a single-writer local CLI; no locking or signature scheme is provided. Integrity checks do not establish real-world truth and do not protect against an operator who can rewrite and rehash the whole store. Query is exact lexical AND matching only. No LLM, embeddings, OCR, semantic extraction or automatic admission to canonical state is included.
