# K501 Information Space — Unified License Policy v1.0

**Policy identifier:** `K501-IS-LICENSE-POLICY-1.0`  
**Owner instruction time anchor:** `1792121537`  
**Prospective scope:** future K501 Information Space packages, unless a specific release file map overrides an asset-class default.

## 1. Policy purpose

This framework is **not a new bespoke omnibus license**. It maps established license texts to asset classes: MIT for eligible K501 software, CC BY 4.0 for eligible authored documentation/conduct, and the applicable upstream license for third-party material. The document `K501 Information Space License Policy v1.0` is a policy/map, not an additional legal license.

K501 is a state-centered information space, not one monolithic software artifact. Its source code, technical prose, conduct rules, source material, generated evidence and third-party tools have different rights boundaries. The license policy therefore uses named licenses by asset class rather than one universal grant that could accidentally relicense material the project does not own.

## 2. Default asset split

- **Software code, build scripts, tests and examples authored or rights-holder-controlled by K501:** MIT (`SPDX-License-Identifier: MIT`). The grant includes normal permission to use, copy, modify, distribute and sublicense within its terms. The copyright and permission notice must travel with substantial copies.
- **K501-authored documentation, technical specifications, data-format definitions, research prose, and code of conduct:** Creative Commons Attribution 4.0 International (`CC-BY-4.0`). Attribution, license reference and change marking apply as specified by that license.
- **Third-party code, formats, source text or assets:** retain the exact upstream license and required attribution. No root-level K501 license overrides a component-level upstream notice.
- **Source corpora, generated user-specific evidence, private material, and personal/confidential data:** no license is granted by default. They remain outside the package unless they receive a specific authorized disclosure and rights assessment.
- **Names, logos and marks:** not granted under MIT/CC BY by default; no implied endorsement.
- **Machine-readable metadata and factual manifests:** covered by the associated file-level map only as text/data rights permit; no license to the underlying referenced files is created by listing their hashes.

## 3. Required package elements

Each public candidate/release must include `LICENSE`, full relevant standard license texts or canonical URI, `LICENSES/FILE_LICENSE_MAP.md`, machine-readable `FILE_LICENSE_MAP.csv`, `THIRD_PARTY_NOTICES.md`, a provenance register, source checksums and a rights-holder scope statement. Missing/unresolved license identity means `G5-C OPEN` and no public redistribution pending resolution.

## 4. Historical and future policy

A current policy can supersede older internal policy notes for prospective work, but cannot pretend historic artifacts never existed, cannot retroactively rewrite grants already received, and cannot revoke third-party rights. Keep previous policy documents in the relevant historical package and record the supersession in a new notice. The exact terms governing a previous copy are established by that copy's actual license files and the rights/licensing law that applies; this framework does not adjudicate disputed ownership.

## 5. Adoption and validation

The project owner is responsible for confirming that they have authority to apply these licenses to each file. A software package build can check that license documents and notices are present; it cannot prove copyright ownership. G5-C is only PASS after the file map, source provenance, component licenses and rights-holder authority are accepted for the intended distribution scope.

The current Alpha 1 implementation uses MIT for code, CC BY 4.0 for authored specification/docs/conduct and no blanket license for personal/local corpora. `LICENSES/FILE_LICENSE_MAP.md` is the controlling per-package map.
