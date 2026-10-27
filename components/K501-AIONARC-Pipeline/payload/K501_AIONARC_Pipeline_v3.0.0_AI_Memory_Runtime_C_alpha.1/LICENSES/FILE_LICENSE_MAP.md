# K501 Information Space — License Map v1.0

**Applies to:** K501-authored or rights-holder-controlled content in this Alpha 1 candidate only.  
**Owner instruction anchor:** Unix `1792121537`.  
**Identifiers:** SPDX `MIT`, `CC-BY-4.0`.  
**Status:** owner-instructed framework, subject to the rights holder's authority over each file; not independent legal advice.

## Asset classes

| Asset class | Default license for K501-owned/controlled A1 content | Included examples | Exclusions |
|---|---|---|---|
| Runtime/source code | MIT | `k501_memory.c`, QH256 API/core files, SHA-256 implementation, code headers | Does not override a separate third-party license if one is identified |
| Build/test/automation code | MIT | `Makefile`, `run.sh`, `scripts/`, `tests/`, `examples/` | External tools invoked but not distributed are not relicensed by this package |
| Human-authored documentation | CC BY 4.0 | `README.md`, `docs/*.md`, Memory Contract, architecture, operator manuals, source register, changelog | Quoted or embedded third-party works retain their source terms |
| Conduct policy | CC BY 4.0 | `CODE_OF_CONDUCT.md` | Does not license trademarks or personal rights |
| Metadata/checksum manifests | CC BY 4.0 for authored descriptive metadata; hash values are factual integrity metadata | `VERSION`, `TIME_ANCHOR.txt`, `PACKAGE_METADATA.json`, `CITATION.cff`, root `NOTICE`, `LICENSE_STATUS.md` | `SHA256SUMS.txt` is an integrity manifest, not a license grant; does not grant rights in files whose hashes are listed |
| K501 policy and provenance notices | CC BY 4.0 for K501-authored text, where rights are held | `LICENSES/*.md`, except the copied standard license texts; `docs/SELF_REVIEW_REPORT.md`, `docs/VALIDATION_REPORT.md` | The policy map does not replace MIT/CC BY legal terms and does not determine copyright ownership |
| Standard license texts | Respective standard license text/source; not a claim of authorship by K501 | `LICENSE`, `LICENSES/MIT.txt`, `LICENSES/CC-BY-4.0.txt` | Canonical upstream license source/terms govern if the convenience copy differs |
| Synthetic PDF test fixture | MIT as package-created test input | `tests/fixtures/minimal-text.pdf` | Uses standard Base-14 Helvetica reference only; no font file is redistributed |
| Third-party software / external tools | Each upstream license applies | `pdftotext`/Poppler when installed on host; system libc | Not bundled or sublicensed here |
| User corpus, local/private files, run logs from another system | `NO_RELICENSE_NO_PUBLICATION` unless explicitly marked | Real sources, personal data, local evidence, user library documents | Not included in this package by default |
| Name, logo, marks and endorsement | Not granted by MIT/CC BY by default | K501 names/branding and any marks | Use must not imply endorsement; trademark rights are reserved |

## Attribution for CC BY 4.0 material

When sharing CC BY 4.0 documentation, retain creator attribution, copyright/license reference and disclaimer notices where supplied, link the license where practicable, and indicate changes. The full license text is in `LICENSES/CC-BY-4.0.txt`; canonical legal code: https://creativecommons.org/licenses/by/4.0/legalcode.

## No blanket relicensing

The file map controls only the Alpha 1 package and only to the extent the rights holder has authority to license each file. An identified upstream license, copyright notice, or non-project source must be retained and correctly described; if it conflicts with this default, mark the asset separately and do not redistribute until its license boundary is resolved. Public availability is not by itself proof of permission.
