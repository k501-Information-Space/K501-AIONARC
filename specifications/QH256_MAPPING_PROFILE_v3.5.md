# QH256 Mapping Profile — Integrated v3.5.0

**Profile ID:** `K501-INTEGRATED-QH256-MAPPING-3.5.0`  
**Status:** aggregate boundary specification; AIONARC cell mapping remains `QH256-K501-AIONARC-MEMORY-V3A1`  
**QH256 core:** v2.0, unchanged

## Mapping authority

The aggregate introduces no new cell assignments. The complete per-cell mapping, assignment rules, reconstruction requirements and caveats remain the table in:

`components/K501-AIONARC-Pipeline/payload/K501_AIONARC_Pipeline_v3.0.0_AI_Memory_Runtime_C_alpha.1/docs/QH256_MAPPING_PROFILE.md`

That profile assigns cells 0–63 for AIONARC Alpha 1 application observations; cells 64–127 remain UNKNOWN/reserved. The code meanings (`UNKNOWN=00`, `FALSE=01`, `TRUE=10`, `GUARD=11`), QH256 v2 merge algebra and 32-byte serialization are not changed by this package.

## Component separation

The Evidence Runtime's process outcomes (`PASS`, `FAIL`, `PARTIAL`) are not mapped into QH256. The recorder has no such application profile, and a child-process result is not equivalent to an AIONARC source observation. Evidence remains in its run record, transcript, result and checksums. A future status mapping requires a separately versioned, reviewed profile with test vectors and an explicit compatibility decision.

## Interpretation limits

QH256 cells encode only the declared profile's evidence about the bounded local scan. They do not establish external truth, semantic correctness, completeness, legal status, gate acceptance or publication. Aggregate values are not substitutes for the underlying frames, objects, text, proof records or recorder runs.
