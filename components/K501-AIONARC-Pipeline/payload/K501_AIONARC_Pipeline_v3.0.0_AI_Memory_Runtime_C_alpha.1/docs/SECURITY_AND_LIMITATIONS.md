# Security and Limitations — Alpha 1

- Single-writer local CLI; no store locking protocol.
- No digital signatures, trusted timestamp authority, remote witness, encryption, or hostile-store guarantees.
- A party able to replace all files and recompute all hashes can create a new internally consistent store.
- Source drift checks are bounded filesystem observations, not proof against every concurrent/adversarial race.
- Data provenance says where a returned result points; it does not prove the source statement is correct.
- PDF text extraction is optional and delegated to an installed `pdftotext`; the child program and its build/license are not distributed by this package.
- Text candidate support is extension-based and bounded by per-file/per-frame byte caps. DOCX is structural-only; no OCR, image, audio, video or model interpretation.
- Query is lexical token AND, not semantic search.
- A failed frame stage is not a frame. Preserve it; do not auto-promote or delete.
- Alpha 1 has not been operator-tested on the user's HP xw8600/Debian workstation by the package preparation process. User-controlled testing remains next.
- The package does not claim G7 reproducibility, universal Linux compatibility, or external scientific validation.
