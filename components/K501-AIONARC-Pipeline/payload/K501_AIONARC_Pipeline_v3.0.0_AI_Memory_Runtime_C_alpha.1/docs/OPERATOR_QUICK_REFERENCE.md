# K501 AIONARC Pipeline v3A1 — Operator Quick Reference

**Version:** `3.0.0-alpha.1`; NON-CANONICAL; do not use real/canonical source trees before operator review.

# Operator Quick Reference

From the package root:

    make all
    make test

Start with a *new* test source directory and *new* empty store:

    build/k501-memory init /path/to/new-store
    build/k501-memory scan /path/to/source /path/to/new-store k501-root frame-001 none
    build/k501-memory verify /path/to/new-store frame-001
    build/k501-memory query /path/to/new-store latest "proof before state"

After modifying or adding source files:

    build/k501-memory scan /path/to/source /path/to/new-store k501-root frame-002 frame-001
    build/k501-memory verify /path/to/new-store frame-002
    build/k501-memory diff /path/to/new-store frame-002
    build/k501-memory verify-source /path/to/new-store frame-002 /path/to/source

For a query hit, copy `OBJECT_ID`:

    build/k501-memory trace /path/to/new-store latest OBJECT_ID
    build/k501-memory history /path/to/new-store OBJECT_ID

Rules: source and store must not overlap; `ROOT_ID` stays stable for one history; frame IDs never repeat; parent is explicit or one unambiguous latest tip; source files are not mutated; query is lexical, not semantic; `PASS` is local contract evidence, not external truth or digital signature. For full steps and failure recovery, read `OPERATOR_MANUAL.md`.

License classes and exclusions: see `LICENSES/FILE_LICENSE_MAP.md`.
