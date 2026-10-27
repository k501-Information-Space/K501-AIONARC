#!/usr/bin/env python3
"""Regenerate the aggregate derived Index, manifest, checksum list and sidecar."""
from __future__ import annotations

import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ARTIFACT_ID = "K501-AIONARC-INTEGRATED-PIPELINE-PROOF-RUNTIME"
VERSION = "3.5.0"
INDEX_EXCLUDED = {"Index.json", "Index.md", "MANIFEST.json", "SHA256SUMS.txt", "SHA256SUMS.txt.sha256"}
MANIFEST_EXCLUDED = {"MANIFEST.json", "SHA256SUMS.txt", "SHA256SUMS.txt.sha256"}
SUMS_EXCLUDED = {"SHA256SUMS.txt", "SHA256SUMS.txt.sha256"}


def included(path: Path) -> bool:
    rel = path.relative_to(ROOT)
    return not any(part in {".git", "build", "__pycache__"} for part in rel.parts)


def root_excluded(path: Path, names: set[str]) -> bool:
    rel = path.relative_to(ROOT)
    return rel.parent == Path(".") and rel.name in names


def digest(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


def files():
    return sorted((p for p in ROOT.rglob("*") if p.is_file() and included(p)), key=lambda p: p.relative_to(ROOT).as_posix())


def entry(path: Path):
    return {"path": path.relative_to(ROOT).as_posix(), "bytes": path.stat().st_size, "sha256": digest(path)}


def write_json(path: Path, value):
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def main():
    current = files()
    index_entries = [entry(p) for p in current if not root_excluded(p, INDEX_EXCLUDED)]
    index = {
        "schema_id": "K501_INTEGRATED_INDEX/1.0",
        "artifact_id": ARTIFACT_ID,
        "version": VERSION,
        "classification": "DERIVED_NAVIGATION_METADATA",
        "scope_note": "Regular files in this candidate tree excluding Index/manifest/checksum recursion and generated build intermediates; paths are relative to artifact root.",
        "entries": index_entries,
    }
    write_json(ROOT / "Index.json", index)
    md = [
        "# Aggregate Index — K501 AIONARC Integrated Pipeline & Proof Runtime 3.5.0",
        "",
        "**Classification:** derived navigation and byte inventory; not a semantic proof.  ",
        f"**Entries:** {len(index_entries)} regular files (generated build directories and recursive integrity files excluded).",
        "",
        "| Path | Bytes | SHA-256 |",
        "|---|---:|---|",
    ]
    md.extend(f"| `{item['path']}` | {item['bytes']} | `{item['sha256']}` |" for item in index_entries)
    (ROOT / "Index.md").write_text("\n".join(md) + "\n", encoding="utf-8")

    manifest_entries = [entry(p) for p in files() if not root_excluded(p, MANIFEST_EXCLUDED)]
    manifest = {
        "schema_id": "K501_INTEGRATED_MANIFEST/1.0",
        "artifact_id": ARTIFACT_ID,
        "version": VERSION,
        "manifest_scope": "All regular files except this manifest and root checksum files; generated component build directories are excluded.",
        "hash_algorithm": "SHA-256",
        "entries": manifest_entries,
    }
    write_json(ROOT / "MANIFEST.json", manifest)

    sum_lines = []
    for path in files():
        if root_excluded(path, SUMS_EXCLUDED):
            continue
        sum_lines.append(f"{digest(path)}  {path.relative_to(ROOT).as_posix()}")
    (ROOT / "SHA256SUMS.txt").write_text("\n".join(sum_lines) + "\n", encoding="utf-8")
    (ROOT / "SHA256SUMS.txt.sha256").write_text(f"{digest(ROOT / 'SHA256SUMS.txt')}  SHA256SUMS.txt\n", encoding="utf-8")
    print(f"INDEX_ENTRIES={len(index_entries)}")
    print(f"MANIFEST_ENTRIES={len(manifest_entries)}")
    print(f"CHECKSUM_ENTRIES={len(sum_lines)}")
    print("INTEGRITY_REFRESH=PASS")


if __name__ == "__main__":
    main()
