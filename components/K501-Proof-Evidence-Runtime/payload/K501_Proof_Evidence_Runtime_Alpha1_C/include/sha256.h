/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Patrick R. Miller (Iinkognit0).
 */
#ifndef K501_SHA256_H
#define K501_SHA256_H
#include <stddef.h>
#include <stdint.h>
typedef struct { uint32_t h[8]; uint64_t bitlen; uint8_t block[64]; size_t used; } sha256_ctx;
void sha256_init(sha256_ctx *ctx);
void sha256_update(sha256_ctx *ctx, const uint8_t *data, size_t len);
void sha256_final(sha256_ctx *ctx, uint8_t out[32]);
void sha256_bytes(const uint8_t *data, size_t len, uint8_t out[32]);
#endif
