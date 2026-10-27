#!/usr/bin/env python3
"""Create a local owner-review ZIP from the verified aggregate manifest."""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
import zipfile

ROOT = Path(__file__).resolve().parents[1]
ARCHIVE = ROOT.parent / f"{ROOT.name}.zip"
SIDECAR = Path(str(ARCHIVE) + ".sha256")


def sha(path: Path):
    h = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


def main():
    if ARCHIVE.exists() or SIDECAR.exists():
        raise SystemExit(f"refusing to overwrite existing release candidate output: {ARCHIVE}")
    manifest = json.loads((ROOT / "MANIFEST.json").read_text(encoding="utf-8"))
    rel_paths = {item["path"] for item in manifest["entries"]}
    rel_paths.update({"MANIFEST.json", "SHA256SUMS.txt", "SHA256SUMS.txt.sha256"})
    root_name = ROOT.name
    with zipfile.ZipFile(ARCHIVE, mode="x", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for rel in sorted(rel_paths):
            path = ROOT / rel
            if not path.is_file():
                raise SystemExit(f"manifest path is missing: {rel}")
            archive.write(path, arcname=f"{root_name}/{rel}")
    digest = sha(ARCHIVE)
    with SIDECAR.open("x", encoding="utf-8") as stream:
        stream.write(f"{digest}  {ARCHIVE.name}\n")
    print(f"ARCHIVE={ARCHIVE}")
    print(f"ARCHIVE_SHA256={digest}")
    print(f"ARCHIVE_BYTES={ARCHIVE.stat().st_size}")
    print("ARCHIVE_STATUS=LOCAL_OWNER_REVIEW_CANDIDATE_NOT_RELEASED")


if __name__ == "__main__":
    main()
