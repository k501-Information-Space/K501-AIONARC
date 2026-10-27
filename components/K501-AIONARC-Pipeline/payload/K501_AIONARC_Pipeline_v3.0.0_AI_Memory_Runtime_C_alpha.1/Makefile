# SPDX-License-Identifier: MIT
CC ?= cc
# Fixed release baseline. Place these options after EXTRA_CFLAGS so -march=native
# supplied through EXTRA_CFLAGS cannot override the qualified x86-64 baseline.
override BASE_CFLAGS := -O2 -Wall -Wextra -Wpedantic -Wno-misleading-indentation -Werror -march=x86-64 -mtune=generic
EXTRA_CFLAGS ?=
CPPFLAGS ?= -Iinclude -Isrc
LDFLAGS ?=
BUILD_DIR := build
TARGET := $(BUILD_DIR)/k501-memory
SOURCES := k501_memory.c src/qh256.c src/sha256.c

.PHONY: all clean test qh256-test sha256-test mapping-test package-check
all: $(TARGET)

$(TARGET): $(SOURCES) include/qh256.h src/sha256.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -std=c11 $(CPPFLAGS) $(EXTRA_CFLAGS) $(BASE_CFLAGS) $(SOURCES) $(LDFLAGS) -o $@

qh256-test: $(TARGET)
	$(TARGET) qh256-test

sha256-test: $(TARGET)
	$(TARGET) sha256-test

mapping-test: $(TARGET)
	$(TARGET) mapping-test

test: $(TARGET)
	bash tests/test.sh "$(abspath $(TARGET))"

package-check:
	bash scripts/verify-package.sh

clean:
	rm -rf $(BUILD_DIR)
