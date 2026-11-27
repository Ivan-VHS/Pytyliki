#include "processor_evidence.h"

#include <string.h>

typedef struct {
    uint32_t state[8];
    uint64_t bit_count;
    unsigned char block[64];
    size_t used;
} sha256_context;

static const uint32_t k_round[64] = {
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

static void transform(sha256_context *ctx, const unsigned char block[64]) {
    uint32_t words[64];
    uint32_t a,b,c,d,e,f,g,h;
    size_t i;
    for (i = 0u; i < 16u; ++i) {
        const size_t p = i * 4u;
        words[i] = ((uint32_t)block[p] << 24u) |
            ((uint32_t)block[p + 1u] << 16u) |
            ((uint32_t)block[p + 2u] << 8u) | (uint32_t)block[p + 3u];
    }
    for (i = 16u; i < 64u; ++i) {
        const uint32_t s0 = rotate_right(words[i - 15u], 7u) ^
            rotate_right(words[i - 15u], 18u) ^ (words[i - 15u] >> 3u);
        const uint32_t s1 = rotate_right(words[i - 2u], 17u) ^
            rotate_right(words[i - 2u], 19u) ^ (words[i - 2u] >> 10u);
        words[i] = words[i - 16u] + s0 + words[i - 7u] + s1;
    }
    a=ctx->state[0]; b=ctx->state[1]; c=ctx->state[2]; d=ctx->state[3];
    e=ctx->state[4]; f=ctx->state[5]; g=ctx->state[6]; h=ctx->state[7];
    for (i = 0u; i < 64u; ++i) {
        const uint32_t s1 = rotate_right(e,6u) ^ rotate_right(e,11u) ^ rotate_right(e,25u);
        const uint32_t choice = (e & f) ^ (~e & g);
        const uint32_t t1 = h + s1 + choice + k_round[i] + words[i];
        const uint32_t s0 = rotate_right(a,2u) ^ rotate_right(a,13u) ^ rotate_right(a,22u);
        const uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
        const uint32_t t2 = s0 + majority;
        h=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
    }
    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

static void initialize(sha256_context *ctx) {
    static const uint32_t initial[8] = {
        0x6a09e667u,0xbb67ae85u,0x3c6ef372u,0xa54ff53au,
        0x510e527fu,0x9b05688cu,0x1f83d9abu,0x5be0cd19u
    };
    memcpy(ctx->state, initial, sizeof initial);
    ctx->bit_count=0u;
    ctx->used=0u;
}

static void update(sha256_context *ctx, const unsigned char *data, size_t size) {
    size_t offset=0u;
    while (offset < size) {
        size_t take=64u-ctx->used;
        if (take > size-offset) take=size-offset;
        memcpy(ctx->block+ctx->used,data+offset,take);
        ctx->used+=take;
        offset+=take;
        if (ctx->used==64u) {
            transform(ctx,ctx->block);
            ctx->bit_count+=512u;
            ctx->used=0u;
        }
    }
}

static void finish(sha256_context *ctx, unsigned char digest[32]) {
    size_t i;
    ctx->bit_count+=(uint64_t)ctx->used*8u;
    ctx->block[ctx->used++]=0x80u;
    if (ctx->used > 56u) {
        while (ctx->used < 64u) ctx->block[ctx->used++]=0u;
        transform(ctx,ctx->block);
        ctx->used=0u;
    }
    while (ctx->used < 56u) ctx->block[ctx->used++]=0u;
    for (i=0u;i<8u;++i)
        ctx->block[63u-i]=(unsigned char)(ctx->bit_count>>(i*8u));
    transform(ctx,ctx->block);
    for (i=0u;i<8u;++i) {
        digest[i*4u]=(unsigned char)(ctx->state[i]>>24u);
        digest[i*4u+1u]=(unsigned char)(ctx->state[i]>>16u);
        digest[i*4u+2u]=(unsigned char)(ctx->state[i]>>8u);
        digest[i*4u+3u]=(unsigned char)ctx->state[i];
    }
}

int pl_tr_sha256_hex(const void *data, size_t size,
                     char output[PL_TR_SHA256_HEX_SIZE]) {
    static const char digits[]="0123456789abcdef";
    sha256_context ctx;
    unsigned char digest[32];
    size_t i;
    if ((data==NULL && size!=0u) || output==NULL) return -1;
    initialize(&ctx);
    if (size!=0u) update(&ctx,(const unsigned char *)data,size);
    finish(&ctx,digest);
    for (i=0u;i<sizeof digest;++i) {
        output[i*2u]=digits[digest[i]>>4u];
        output[i*2u+1u]=digits[digest[i]&0x0fu];
    }
    output[64]='\0';
    return 0;
}
