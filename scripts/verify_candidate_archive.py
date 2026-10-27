#!/usr/bin/env python3
"""Read-only verification of the generated local candidate ZIP."""
from __future__ import annotations

import hashlib
import json
from pathlib import Path, PurePosixPath
import sys
import zipfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    if len(sys.argv) != 2:
        print("usage: verify_candidate_archive.py /absolute/path/to/archive.zip", file=sys.stderr)
        return 64
    archive_path = Path(sys.argv[1]).resolve(strict=True)
    manifest = json.loads((ROOT / "MANIFEST.json").read_text(encoding="utf-8"))
    declared = {item["path"]: item for item in manifest["entries"]}
    expected = set(declared) | {"MANIFEST.json", "SHA256SUMS.txt", "SHA256SUMS.txt.sha256"}
    root_name = ROOT.name
    seen = {}
    with zipfile.ZipFile(archive_path, "r") as archive:
        for info in archive.infolist():
            path = PurePosixPath(info.filename)
            if path.is_absolute() or ".." in path.parts or len(path.parts) < 2 or path.parts[0] != root_name or info.is_dir():
                raise SystemExit(f"unsafe or unexpected ZIP member: {info.filename}")
            if info.filename in seen:
                raise SystemExit(f"duplicate ZIP member: {info.filename}")
            rel = PurePosixPath(*path.parts[1:]).as_posix()
            raw = archive.read(info)
            seen[rel] = hashlib.sha256(raw).hexdigest()
            if rel in declared:
                item = declared[rel]
                if len(raw) != item["bytes"] or seen[rel] != item["sha256"]:
                    raise SystemExit(f"manifest mismatch in archive: {rel}")
    if set(seen) != expected:
        raise SystemExit(f"ZIP path set differs: missing={sorted(expected-set(seen))[:5]} extra={sorted(set(seen)-expected)[:5]}")
    print(f"ARCHIVE_MEMBERS_PASS files={len(seen)}")
    print("ARCHIVE_TREE_MANIFEST_PARITY=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
