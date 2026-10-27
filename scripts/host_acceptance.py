#!/usr/bin/env python3
"""Isolated host acceptance for the documented aggregate quickstart."""
from __future__ import annotations
import hashlib, json, platform, subprocess, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
MEM=ROOT/"dist/linux-x86_64/k501-memory"
REC=ROOT/"dist/linux-x86_64/k501-proof-evidence"
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args, expect=0):
    p=subprocess.run([str(x) for x in args], text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if (expect is None and p.returncode == 0) or (expect is not None and p.returncode != expect): raise RuntimeError(f"exit={p.returncode}, expected={expect}: {args}\n{p.stdout}")
    return p.stdout

def main():
    if platform.system()!="Linux" or platform.machine() not in ("x86_64","amd64"):
        raise SystemExit("HOST_ACCEPTANCE=SKIP_UNSUPPORTED_HOST")
    with tempfile.TemporaryDirectory(prefix="k501-v350-host-") as td:
        t=Path(td); src=t/"source"; src.mkdir(); (src/"README.md").write_text("proof before state\n")
        store=t/"store"; run([MEM,"init",store]); run([MEM,"scan",src,store,"k501-v35-host","frame-001","none"])
        verify=run([MEM,"verify",store,"frame-001"]); query=run([MEM,"query",store,"latest","proof before state"])
        overlap_store=t/"overlap"; run([MEM,"init",overlap_store])
        overlap=run([MEM,"scan",overlap_store,overlap_store,"k501-v35-host","frame-overlap","none"],expect=None)
        evidence=t/"evidence-workspace"; evidence.mkdir()
        record=run([REC,"run",evidence,"system","host-acceptance","--","/usr/bin/wc","-c",src/"README.md"])
        produced=list(evidence.rglob("EVIDENCE_RECORD.json"))
        if len(produced)!=1: raise RuntimeError(f"expected one recorder evidence record, found {len(produced)}")
        parsed=json.loads(produced[0].read_text())
        if parsed.get("outcome",{}).get("gate_effect")!="NO_GATE_EFFECT_UNTIL_REVIEW": raise RuntimeError("unexpected recorder gate_effect")
        print("HOST=Linux x86_64")
        print(f"MEMORY_SHA256={sha(MEM)}")
        print(f"RECORDER_SHA256={sha(REC)}")
        print("QUICKSTART_INIT_SCAN_VERIFY_QUERY=PASS")
        print("NEGATIVE_OVERLAP_REJECTION=PASS")
        print("RECORDER_CHILD_CAPTURE_AND_EVIDENCE_RECORD=PASS")
        print("HOST_ACCEPTANCE=PASS")
if __name__=="__main__": main()
