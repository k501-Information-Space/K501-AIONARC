#!/usr/bin/env python3
"""Build two isolated copies of each pinned component source and compare outputs."""
from __future__ import annotations
import hashlib, json, platform, shutil, subprocess, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
SOURCES=[
 ("AIONARC",ROOT/"components/K501-AIONARC-Pipeline/payload/K501_AIONARC_Pipeline_v3.0.0_AI_Memory_Runtime_C_alpha.1","build/k501-memory",ROOT/"dist/linux-x86_64/k501-memory"),
 ("Proof-Evidence-Runtime",ROOT/"components/K501-Proof-Evidence-Runtime/payload/K501_Proof_Evidence_Runtime_Alpha1_C","build/k501-proof-evidence",ROOT/"dist/linux-x86_64/k501-proof-evidence"),
]
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def build(src, out):
    subprocess.run(["make","clean"],cwd=src,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    subprocess.run(["make","all"],cwd=src,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    return src/out
def main():
    rows=[]
    with tempfile.TemporaryDirectory(prefix="k501-v350-repro-") as td:
        base=Path(td)
        for name,source,rel,current in SOURCES:
            pair=[]
            for copy_no in ("A","B"):
                target=base/name/copy_no/source.name
                target.parent.mkdir(parents=True,exist_ok=True)
                shutil.copytree(source,target,ignore=shutil.ignore_patterns("build","__pycache__"))
                binary=build(target,Path(rel))
                pair.append(sha(binary))
            current_hash=sha(current)
            if pair[0]!=pair[1] or pair[0]!=current_hash:
                raise SystemExit(f"REPRODUCIBILITY_FAIL {name} clean_a={pair[0]} clean_b={pair[1]} aggregate={current_hash}")
            rows.append({"component":name,"clean_copy_A_sha256":pair[0],"clean_copy_B_sha256":pair[1],"aggregate_sha256":current_hash,"match":True})
    print(json.dumps({"host":{"system":platform.system(),"machine":platform.machine()},"scope":"two independent clean copies of pinned component source trees, same host and toolchain","components":rows,"result":"PASS"},indent=2))
    print("REPRODUCIBILITY=PASS")
if __name__=="__main__": main()
