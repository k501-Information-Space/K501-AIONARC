#!/usr/bin/env python3
"""Calculate release profile status from gate records; never changes gate state."""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    profile = json.loads((ROOT / "specifications/RELEASE_GATE_PROFILE_v3.5.json").read_text(encoding="utf-8"))
    status = json.loads((ROOT / "metadata/GATE_STATUS.json").read_text(encoding="utf-8"))
    gate_status = {gate_id: record["status"] for gate_id, record in status["gates"].items()}
    allowed = set(profile["status_vocabulary"])
    errors = []
    for gate in profile["gates"]:
        actual = gate_status.get(gate["gate_id"])
        if actual not in allowed:
            errors.append(f"{gate['gate_id']}: missing/invalid status {actual!r}")
    for release_profile in profile["release_profiles"]:
        blockers = [gate_id for gate_id in release_profile["required_gates"] if gate_status.get(gate_id) != "PASS"]
        result = "READY_FOR_REVIEW" if not blockers else "BLOCKED"
        print(f"{release_profile['profile_id']}={result}")
        if blockers:
            print("  blockers=" + ",".join(blockers))
    if errors:
        for error in errors:
            print("FAIL " + error)
        return 1
    print("GATE_CALCULATION=PASS (calculation only; no gate was promoted)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
