#!/usr/bin/env python3
"""Verify package and aggregate identities without changing artifact files."""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
EXCLUDED = {"MANIFEST.json", "SHA256SUMS.txt", "SHA256SUMS.txt.sha256"}


def files():
    return sorted((p for p in ROOT.rglob("*") if p.is_file() and not any(x in {".git", "build", "__pycache__"} for x in p.relative_to(ROOT).parts)), key=lambda p: p.relative_to(ROOT).as_posix())


def is_root_integrity(path: Path) -> bool:
    return path.parent == ROOT and path.name in EXCLUDED


def sha(path: Path):
    h = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


def check_sum_file(cwd: Path, filename: str):
    subprocess.run(["sha256sum", "-c", filename], cwd=cwd, check=True, stdout=subprocess.DEVNULL)


def main():
    errors = []
    # Validate all preserved project-root and payload checksum sets.
    projects = [ROOT / "components/K501-AIONARC-Pipeline", ROOT / "components/K501-Proof-Evidence-Runtime"]
    payloads = [
        projects[0] / "payload/K501_AIONARC_Pipeline_v3.0.0_AI_Memory_Runtime_C_alpha.1",
        projects[1] / "payload/K501_Proof_Evidence_Runtime_Alpha1_C",
    ]
    for path in projects + payloads:
        try:
            check_sum_file(path, "SHA256SUMS.txt")
            print(f"COMPONENT_SUMS_PASS {path.relative_to(ROOT)}")
        except Exception as exc:
            errors.append(f"component sums failed at {path.relative_to(ROOT)}: {exc}")

    try:
        check_sum_file(ROOT, "SHA256SUMS.txt")
        check_sum_file(ROOT, "SHA256SUMS.txt.sha256")
        print("ROOT_CHECKSUMS_PASS")
    except Exception as exc:
        errors.append(f"root checksum verification failed: {exc}")

    try:
        manifest = json.loads((ROOT / "MANIFEST.json").read_text(encoding="utf-8"))
        actual = {p.relative_to(ROOT).as_posix(): p for p in files() if not is_root_integrity(p)}
        declared = {item["path"]: item for item in manifest["entries"]}
        if set(actual) != set(declared):
            errors.append(f"manifest path set differs: missing={sorted(set(actual)-set(declared))[:5]} extra={sorted(set(declared)-set(actual))[:5]}")
        for rel, path in actual.items():
            item = declared.get(rel)
            if item and (item["bytes"] != path.stat().st_size or item["sha256"] != sha(path)):
                errors.append(f"manifest digest mismatch: {rel}")
        print(f"ROOT_MANIFEST_PASS entries={len(declared)}")
    except Exception as exc:
        errors.append(f"root manifest invalid: {exc}")

    try:
        index = json.loads((ROOT / "Index.json").read_text(encoding="utf-8"))
        index_excluded = {"Index.json", "Index.md", "MANIFEST.json", "SHA256SUMS.txt", "SHA256SUMS.txt.sha256"}
        expected = {p.relative_to(ROOT).as_posix(): p for p in files() if not (p.parent == ROOT and p.name in index_excluded)}
        declared = {item["path"]: item for item in index["entries"]}
        if set(expected) != set(declared):
            errors.append("derived Index path set is stale")
        for rel, path in expected.items():
            item = declared.get(rel)
            if item and (item["bytes"] != path.stat().st_size or item["sha256"] != sha(path)):
                errors.append(f"Index digest mismatch: {rel}")
        print(f"INDEX_PASS entries={len(declared)}")
    except Exception as exc:
        errors.append(f"Index invalid: {exc}")

    try:
        identity = json.loads((ROOT / "metadata/ARTIFACT_IDENTITY.json").read_text(encoding="utf-8"))
        if identity["version"] != "3.5.0" or identity["artifact_id"] != "K501-AIONARC-INTEGRATED-PIPELINE-PROOF-RUNTIME":
            errors.append("aggregate identity/version mismatch")
        expected = {
            "K501-AIONARC-PIPELINE": "f14e203ad7bfa466d118780b06b69e59198e2fd917cb0e7e82222b83557df607",
            "K501-PROOF-EVIDENCE-RUNTIME": "c48fd8863b3b5b8889f46af1efbc768281490b60ab4433700b1331793dfcf435",
        }
        got = {component["project_id"]: component["source_archive_sha256"] for component in identity["components"]}
        if any(got.get(project) != digest_value for project, digest_value in expected.items()):
            errors.append("component source archive identities do not match the declared candidate digests")
        for rel in ["dist/linux-x86_64/k501-memory", "dist/linux-x86_64/k501-proof-evidence"]:
            if not (ROOT / rel).is_file():
                errors.append(f"missing staged executable {rel}")
        binaries = [
            (ROOT / "dist/linux-x86_64/k501-memory", ROOT / "components/K501-AIONARC-Pipeline/payload/K501_AIONARC_Pipeline_v3.0.0_AI_Memory_Runtime_C_alpha.1/build/k501-memory"),
            (ROOT / "dist/linux-x86_64/k501-proof-evidence", ROOT / "components/K501-Proof-Evidence-Runtime/payload/K501_Proof_Evidence_Runtime_Alpha1_C/build/k501-proof-evidence"),
        ]
        for staged, built in binaries:
            if not staged.is_file() or not built.is_file() or sha(staged) != sha(built):
                errors.append(f"staged executable differs from same-tree build: {staged.relative_to(ROOT)}")
            else:
                print(f"BINARY_IDENTITY_PASS {staged.relative_to(ROOT)} sha256={sha(staged)}")
    except Exception as exc:
        errors.append(f"aggregate identity/binary validation failed: {exc}")

    validator = ROOT / "scripts/validate_contracts.py"
    try:
        subprocess.run([sys.executable, str(validator)], cwd=ROOT, check=True)
    except Exception as exc:
        errors.append(f"contract validation failed: {exc}")

    if errors:
        for error in errors:
            print(f"FAIL {error}", file=sys.stderr)
        print("INTEGRATED_ARTIFACT_VERIFY=FAIL")
        return 1
    print("INTEGRATED_ARTIFACT_VERIFY=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
