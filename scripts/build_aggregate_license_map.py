#!/usr/bin/env python3
"""Build the path-complete license map from the owner-ratified aggregate policy."""
from __future__ import annotations

import csv
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MAP_OUT = ROOT / "LICENSES/AGGREGATE_FILE_LICENSE_MAP.json"
COMPONENT_MAPS = {
    "K501-AIONARC-Pipeline": ROOT / "components/K501-AIONARC-Pipeline/LICENSES/FILE_LICENSE_MAP.csv",
    "K501-Proof-Evidence-Runtime": ROOT / "components/K501-Proof-Evidence-Runtime/LICENSES/FILE_LICENSE_MAP.csv",
}
EXCLUDED_PARTS = {".git", "build", "__pycache__"}
ROOT_INTEGRITY = {"MANIFEST.json", "Index.json", "Index.md", "SHA256SUMS.txt", "SHA256SUMS.txt.sha256"}


def from_component(rel: str):
    for component, map_path in COMPONENT_MAPS.items():
        prefix = f"components/{component}/"
        if not rel.startswith(prefix):
            continue
        source_path = rel[len(prefix):]
        with map_path.open(encoding="utf-8", newline="") as stream:
            rows = {row["path"]: row for row in csv.DictReader(stream)}
        row = rows.get(source_path)
        if not row:
            return None
        old = row["disposition"]
        if old.startswith("K501-CC-BY-4.0"):
            return "documentation_or_metadata", "CC-BY-4.0", "OWNER_DECLARED", "Owner declaration 1793046600; third-party exclusions preserved."
        if old.startswith("K501-MIT"):
            return "software_or_code", "MIT", "OWNER_DECLARED", "Owner declaration 1793046600; existing file-level notices preserved."
        if old == "HOLD/UNRESOLVED":
            if source_path == ".gitignore":
                return "software_configuration", "MIT", "OWNER_DECLARED", "Classified as software/build configuration by the owner-ratified policy."
            if source_path == "Gateway.json":
                return "descriptive_metadata", "CC-BY-4.0", "OWNER_DECLARED", "Classified as authored descriptive metadata by the owner-ratified policy."
            if source_path == "releases/RELEASE_POLICY.md":
                return "documentation", "CC-BY-4.0", "OWNER_DECLARED", "Classified as authored release documentation by the owner-ratified policy."
            return None
        if old == "LICENSE_SCOPE_ROUTER":
            return "license_scope_document", "CC-BY-4.0", "OWNER_DECLARED", "Authored scope document; not a blanket license grant."
        if old == "STANDARD-LICENSE-TEXT":
            name = "MIT" if "MIT" in source_path else "CC-BY-4.0" if "CC-BY" in source_path else "NOASSERTION"
            return "standard_license_text", "NOASSERTION", "PRESERVED_STANDARD_TEXT", f"Standard {name} text; no ownership claim."
        if old.startswith("FACTUAL") or old == "FACTUAL/INTEGRITY-METADATA":
            return "factual_or_integrity_record", "NOASSERTION", "FACTUAL_METADATA", "Factual/integrity record; no blanket copyright license asserted."
        if old == "PRESERVED_UPSTREAM" or "UPSTREAM" in old:
            spdx = row.get("spdx", "NOASSERTION")
            return "third_party_or_upstream", spdx, "PRESERVED_UPSTREAM", "Preserve the source notice and terms; not re-licensed by this ratification."
        return None
    return None


def classify(rel: str):
    if rel.startswith("components/"):
        if rel.endswith(("/MANIFEST.json", "/Index.json", "/Index.md", "/SHA256SUMS.txt", "/SHA256SUMS.txt.sha256")):
            return "integrity_or_index_record", "NOASSERTION", "INTEGRITY_METADATA", "Generated index or integrity data; no license assertion over factual digests."
        return from_component(rel)
    if rel.startswith("evidence/runs/"):
        return "controlled_run_evidence", "NOASSERTION", "FACTUAL_EVIDENCE", "Run transcript/metadata/checksums; retain evidence scope and notices."
    if rel in ROOT_INTEGRITY or rel.endswith("/MANIFEST.json") or rel.endswith("/Index.json") or rel.endswith("/Index.md") or rel.endswith("/SHA256SUMS.txt") or rel.endswith("/SHA256SUMS.txt.sha256"):
        return "integrity_or_index_record", "NOASSERTION", "INTEGRITY_METADATA", "Generated index or integrity data; no license assertion over factual digests."
    if rel.startswith("dist/"):
        return "software_binary", "MIT", "OWNER_DECLARED", "Executable built from owner-declared MIT software sources."
    if rel.startswith("scripts/") or rel == "Makefile" or rel == ".gitignore":
        return "software_or_build_code", "MIT", "OWNER_DECLARED", "Owner declaration 1793046600."
    if rel.startswith("LICENSES/") and rel.endswith(("MIT.txt", "CC-BY-4.0.txt")):
        return "standard_license_text", "NOASSERTION", "PRESERVED_STANDARD_TEXT", "Standard license text/reference; no ownership claim."
    if rel in {"Gateway.md", "Gateway.json"}:
        return "descriptive_metadata", "NOASSERTION", "FACTUAL_METADATA", "Repository navigation metadata; no copyright ownership or license inferred."
    if rel == "VERSION":
        return "version_fact", "NOASSERTION", "FACTUAL_METADATA", "Version identifier."
    if rel.startswith(("docs/", "specifications/", "schemas/", "validation/", "zenodo/", "metadata/", "LICENSES/")) or rel in {"README.md", "CHANGELOG.md", "CITATION.cff", "NOTICE", "LICENSE", "LICENSE_SCOPE.md", "LICENSE_STATUS.md", "STATUS.md"}:
        return "documentation_specification_or_metadata", "CC-BY-4.0", "OWNER_DECLARED", "Owner declaration 1793046600; any third-party material remains separately scoped."
    return None


def main():
    paths = sorted((p.relative_to(ROOT).as_posix() for p in ROOT.rglob("*") if p.is_file() and not (set(p.relative_to(ROOT).parts) & EXCLUDED_PARTS)), key=str)
    # These two outputs are known entries even on the first generation pass.
    for rel in ("LICENSES/AGGREGATE_FILE_LICENSE_MAP.json", "LICENSES/AGGREGATE_FILE_LICENSE_MAP.csv"):
        if rel not in paths:
            paths.append(rel)
    entries = []
    unresolved = []
    for rel in sorted(set(paths)):
        row = classify(rel)
        if row is None:
            unresolved.append(rel)
            continue
        kind, spdx, status, note = row
        entries.append({"path": rel, "kind": kind, "spdx": spdx, "status": status, "basis": "metadata/RIGHTSHOLDER_ATTESTATION.json" if status == "OWNER_DECLARED" else status, "scope_note": note})
    if unresolved:
        print("UNRESOLVED_LICENSE_PATHS=" + ",".join(unresolved))
        return 1
    document = {
        "schema_id": "K501_AGGREGATE_FILE_LICENSE_MAP/1.0",
        "artifact_id": "K501-AIONARC-INTEGRATED-PIPELINE-PROOF-RUNTIME",
        "artifact_version": "3.5.0",
        "attestation": "metadata/RIGHTSHOLDER_ATTESTATION.json",
        "default_rule": "NOASSERTION; every distributable path is explicitly classified; preserve upstream notices and terms.",
        "entry_count": len(entries),
        "entries": entries,
    }
    MAP_OUT.write_text(json.dumps(document, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    with (ROOT / "LICENSES/AGGREGATE_FILE_LICENSE_MAP.csv").open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=["path", "kind", "spdx", "status", "basis", "scope_note"])
        writer.writeheader()
        writer.writerows(entries)
    print(f"AGGREGATE_LICENSE_MAP_ENTRIES={len(entries)}")
    print("AGGREGATE_LICENSE_MAP_UNRESOLVED=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
