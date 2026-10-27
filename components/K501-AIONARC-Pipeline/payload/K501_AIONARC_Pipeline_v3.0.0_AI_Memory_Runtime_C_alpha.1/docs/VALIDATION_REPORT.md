# Alpha 1 Build-Time Validation Report

**Package:** `3.0.0-alpha.1`  
**Status:** package-local engineering validation PASS; not canonical and not yet operator-host validated.  
**Validation boundary:** extracted Alpha 1 candidate in the packaging environment. These results do not substitute for the user's controlled unpack/run evidence.

## Executed commands and observed results

The following commands completed successfully after the last source/test changes:

- `make clean all` — compiler returned exit zero under the declared generic x86-64 baseline flags.
- `build/k501-memory qh256-test` — `QH256 CORE TEST PASS`.
- `build/k501-memory sha256-test` — `SHA256 TEST PASS` for empty input, `abc`, and incremental update.
- `build/k501-memory mapping-test` — `QH256 MAPPING PROFILE V3A1 TEST PASS` for mapping outcomes.
- `make test` — `K501 V3A1 TEST SUITE: PASS`.
- Full `make test` was additionally rebuilt and executed with AddressSanitizer and UndefinedBehaviorSanitizer flags (`-fsanitize=address,undefined -fno-omit-frame-pointer`) and leak detection enabled — PASS with no sanitizer diagnostics.

The full suite covers initial/subsequent frames; proof-before-state; structural source parity; query, trace, history and diff; invalid UTF-8; embedded NUL; empty-text row with trailing empty Base64 field; per-file limit; optional PDF extraction present/absent; QH256 status reconstruction; tampered index; source drift; repeated frame IDs; ambiguous branch tips; Alpha 0 store refusal; and both source/store overlap directions.

## Build and runtime qualification

Observed package-building environment: Linux x86-64, kernel `6.18.44`, glibc `2.41`, `cc (Debian 14.2.0-19) 14.2.0`, GNU Make `4.4.1`. Final packaged binary identity: SHA-256 `838746254724916d60c53fd1de5cf112938271b991e127e9a29bab547d019ff2`; ELF Build ID `62300232babfa51027eb03b2de290694a459ded0`. ELF metadata reports x86-64 baseline, interpreter `/lib64/ld-linux-x86-64.so.2`, direct `NEEDED` entry `libc.so.6`, and no RPATH/RUNPATH. Required GLIBC symbol versions extend through `GLIBC_2.34`. This is not a universal Linux portability or future-Debian guarantee.

The Makefile fixes `-march=x86-64 -mtune=generic` after `EXTRA_CFLAGS`; it retains the existing `-Wno-misleading-indentation` suppression. That suppression is disclosed rather than presented as a fully warning-clean source-style pass. It does not suppress `-Wall`, `-Wextra`, or `-Wpedantic` generally, and `-Werror` remains enabled for non-suppressed warnings.

## Not claimed

- No successful user-host/HP xw8600 test is claimed by this report.
- No G5-A formal closure is claimed until the exact delivered archive passes the user's controlled series and owner reviews the evidence.
- No G5-C legal clearance or independent legal opinion is claimed. File-level rights-holder scope review remains required.
- No G7 reproducible-build result is claimed.
- No public repository write, Git tag, GitHub release, or Zenodo upload has occurred.

Final archive SHA-256 values are in the external sidecar files accompanying the delivered archives, and internal file SHA-256 values are recorded in `SHA256SUMS.txt`.
