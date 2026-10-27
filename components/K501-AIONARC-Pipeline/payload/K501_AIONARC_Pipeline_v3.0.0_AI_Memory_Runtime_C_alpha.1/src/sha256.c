/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Patrick R. Miller (Iinkognit0).
 * SHA-256 implementation for the K501 Alpha 1 runtime, written against
 * the standard algorithm specified in NIST FIPS 180-4.
 */
#include "sha256.h"
#include <string.h>

static const uint32_t round_constants[64] = {
    0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
    0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
    0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
    0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
    0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
    0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
    0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
    0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u
};

static uint32_t rotate_right(uint32_t value, unsigned count) {
    return (value >> count) | (value << (32u - count));
}
static uint32_t choose(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (~x & z);
}
static uint32_t majority(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}
static uint32_t big_sigma0(uint32_t x) {
    return rotate_right(x, 2u) ^ rotate_right(x, 13u) ^ rotate_right(x, 22u);
}
static uint32_t big_sigma1(uint32_t x) {
    return rotate_right(x, 6u) ^ rotate_right(x, 11u) ^ rotate_right(x, 25u);
}
static uint32_t small_sigma0(uint32_t x) {
    return rotate_right(x, 7u) ^ rotate_right(x, 18u) ^ (x >> 3u);
}
static uint32_t small_sigma1(uint32_t x) {
    return rotate_right(x, 17u) ^ rotate_right(x, 19u) ^ (x >> 10u);
}
static uint32_t read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24u) | ((uint32_t)p[1] << 16u) |
           ((uint32_t)p[2] << 8u) | (uint32_t)p[3];
}
static void write_be32(uint8_t *p, uint32_t value) {
    p[0]=(uint8_t)(value >> 24u); p[1]=(uint8_t)(value >> 16u);
    p[2]=(uint8_t)(value >> 8u); p[3]=(uint8_t)value;
}
static void compress_block(sha256_ctx *ctx, const uint8_t block[64]) {
    uint32_t words[64];
    uint32_t a,b,c,d,e,f,g,h;
    for (size_t i=0;i<16u;i++) words[i]=read_be32(block+4u*i);
    for (size_t i=16u;i<64u;i++)
        words[i]=small_sigma1(words[i-2u])+words[i-7u]+small_sigma0(words[i-15u])+words[i-16u];
    a=ctx->h[0]; b=ctx->h[1]; c=ctx->h[2]; d=ctx->h[3];
    e=ctx->h[4]; f=ctx->h[5]; g=ctx->h[6]; h=ctx->h[7];
    for (size_t i=0;i<64u;i++) {
        uint32_t t1=h+big_sigma1(e)+choose(e,f,g)+round_constants[i]+words[i];
        uint32_t t2=big_sigma0(a)+majority(a,b,c);
        h=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
    }
    ctx->h[0]+=a; ctx->h[1]+=b; ctx->h[2]+=c; ctx->h[3]+=d;
    ctx->h[4]+=e; ctx->h[5]+=f; ctx->h[6]+=g; ctx->h[7]+=h;
}
void sha256_init(sha256_ctx *ctx) {
    ctx->h[0]=0x6a09e667u; ctx->h[1]=0xbb67ae85u; ctx->h[2]=0x3c6ef372u; ctx->h[3]=0xa54ff53au;
    ctx->h[4]=0x510e527fu; ctx->h[5]=0x9b05688cu; ctx->h[6]=0x1f83d9abu; ctx->h[7]=0x5be0cd19u;
    ctx->bitlen=0u; ctx->used=0u;
}
void sha256_update(sha256_ctx *ctx, const uint8_t *data, size_t len) {
    while (len>0u) {
        size_t take=64u-ctx->used;
        if (take>len)take=len;
        memcpy(ctx->block+ctx->used,data,take);
        ctx->used+=take; data+=take; len-=take;
        ctx->bitlen+=(uint64_t)take*8u;
        if (ctx->used==64u) { compress_block(ctx,ctx->block); ctx->used=0u; }
    }
}
void sha256_final(sha256_ctx *ctx, uint8_t out[32]) {
    uint64_t original_bitlen=ctx->bitlen;
    ctx->block[ctx->used++]=0x80u;
    if(ctx->used>56u){
        while(ctx->used<64u)ctx->block[ctx->used++]=0u;
        compress_block(ctx,ctx->block);ctx->used=0u;
    }
    while(ctx->used<56u)ctx->block[ctx->used++]=0u;
    for(size_t i=0;i<8u;i++)ctx->block[56u+i]=(uint8_t)(original_bitlen>>(56u-8u*i));
    compress_block(ctx,ctx->block);
    for(size_t i=0;i<8u;i++)write_be32(out+4u*i,ctx->h[i]);
}
void sha256_bytes(const uint8_t *data,size_t len,uint8_t out[32]) {
    sha256_ctx ctx;sha256_init(&ctx);sha256_update(&ctx,data,len);sha256_final(&ctx,out);
}
