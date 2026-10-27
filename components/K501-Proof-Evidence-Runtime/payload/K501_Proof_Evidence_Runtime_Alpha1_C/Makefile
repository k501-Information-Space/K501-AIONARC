# K501 Proof and Evidence Runtime Alpha 1 — native C11 build.
CC ?= cc
CPPFLAGS ?= -Iinclude
CFLAGS ?= -O2 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror
CFLAGS += -std=c11
LDFLAGS ?=
BUILD_DIR := build
APP := $(BUILD_DIR)/k501-proof-evidence
TEST := $(BUILD_DIR)/test-runtime
RUNTIME_BIN ?=
SOURCES := src/k501_proof_evidence.c src/qh256.c src/sha256.c

.PHONY: all test clean verify
all: $(APP)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(APP): $(SOURCES) include/qh256.h include/sha256.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SOURCES) $(LDFLAGS) -o $@

$(TEST): tests/test_runtime.c include/qh256.h include/sha256.h src/qh256.c src/sha256.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_runtime.c src/qh256.c src/sha256.c $(LDFLAGS) -o $@

test: $(APP) $(TEST)
	$(TEST) $(APP) $(RUNTIME_BIN)

verify: $(APP)
	$(APP) self-test

clean:
	rm -f $(APP) $(TEST)
