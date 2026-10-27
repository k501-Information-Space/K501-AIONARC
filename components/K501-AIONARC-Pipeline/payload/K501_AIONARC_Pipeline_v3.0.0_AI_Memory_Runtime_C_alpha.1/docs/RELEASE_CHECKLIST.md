# Alpha 1 Release / Canonization Checklist

This list is intentionally not pre-marked as passed.

- [ ] Unpack the received Alpha 1 archive into a new, isolated, previously absent directory.
- [ ] Verify the archive-side SHA-256 before extraction.
- [ ] Verify all package file checksums after extraction.
- [ ] Record OS, kernel, architecture, glibc, compiler and Make versions.
- [ ] Run package `make test` or `scripts/verify-package.sh` through the controlled operator wrapper.
- [ ] Inspect and preserve the full command/output/status/manifest evidence.
- [ ] Inspect ELF ISA baseline, direct dependencies, interpreter, RPATH/RUNPATH and required GLIBC symbol versions.
- [ ] Confirm the actual host satisfies the qualified ABI floor or rebuild locally with the fixed release baseline.
- [ ] Review the documented `-Wno-misleading-indentation` exception; do not imply it has been removed.
- [ ] Test with `pdftotext` available and unavailable.
- [ ] Verify captured-empty, invalid UTF-8, embedded NUL, file-limit and PDF failures.
- [ ] Confirm Alpha 0 paths/hashes have not changed.
- [ ] Review the Memory Contract and license/file map as rights holder.
- [ ] Resolve G5-A against the recorded tests, G5-C against component-by-component license scope, and G7 as a separate reproducible-build gate.
- [ ] Obtain separate explicit approval before any public repository write, tag, GitHub release, or Zenodo upload.
