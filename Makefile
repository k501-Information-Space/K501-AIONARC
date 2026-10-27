SHELL := /bin/sh
.DEFAULT_GOAL := all

AIONARC_DIR := components/K501-AIONARC-Pipeline/payload/K501_AIONARC_Pipeline_v3.0.0_AI_Memory_Runtime_C_alpha.1
EVIDENCE_DIR := components/K501-Proof-Evidence-Runtime/payload/K501_Proof_Evidence_Runtime_Alpha1_C
AIONARC_BIN := $(AIONARC_DIR)/build/k501-memory
EVIDENCE_BIN := $(EVIDENCE_DIR)/build/k501-proof-evidence
DIST_DIR := dist/linux-x86_64

.PHONY: all build test verify package-check clean refresh-integrity

all: build

build:
	$(MAKE) -C "$(AIONARC_DIR)" clean all
	$(MAKE) -C "$(EVIDENCE_DIR)" clean all
	mkdir -p "$(DIST_DIR)"
	cp "$(AIONARC_BIN)" "$(DIST_DIR)/k501-memory"
	cp "$(EVIDENCE_BIN)" "$(DIST_DIR)/k501-proof-evidence"

test: build
	$(MAKE) -C "$(AIONARC_DIR)" qh256-test sha256-test mapping-test test
	$(MAKE) -C "$(EVIDENCE_DIR)" test RUNTIME_BIN="$(abspath $(AIONARC_BIN))"
	"$(EVIDENCE_BIN)" self-test
	python3 scripts/validate_contracts.py

verify: test package-check
	python3 scripts/verify_integrated_artifact.py

package-check:
	bash "$(AIONARC_DIR)/scripts/verify-package.sh"
	(cd "$(EVIDENCE_DIR)" && sha256sum -c SHA256SUMS.txt)
	"$(EVIDENCE_BIN)" self-test

refresh-integrity:
	python3 scripts/refresh_integrity.py

clean:
	$(MAKE) -C "$(AIONARC_DIR)" clean
	$(MAKE) -C "$(EVIDENCE_DIR)" clean
	rm -rf build "$(DIST_DIR)"
