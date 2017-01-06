#ifndef SRLXBOOT_INTRINSICS_H
#define SRLXBOOT_INTRINSICS_H

#include <stdint.h>
#include <stddef.h>

#if SIZE_MAX == UINT64_MAX
#define SIZE_WIDTH 64
#elif SIZE_MAX == UINT32_MAX
#define SIZE_WIDTH 32
#else
#error "unsupported target"
#endif

#ifdef __GNUC__
#define CountLeadingZeros64(x) __builtin_clzll(x)
#define CountLeadingZeros32(x) __builtin_clz(x)
#elif defined(_MSC_VER)
#include <intrin.h>
#define CountLeadingZeros64(x) (int)__lzcnt64(x)
#define CountLeadingZeros32(x) (int)__lzcnt(x)
#else
#error "unsupported compiler"
#endif

#if SIZE_WIDTH == 64
#define CountLeadingZerosN(x) CountLeadingZeros64(x)
#else
#define CountLeadingZerosN(x) CountLeadingZeros32(x)
#endif

#endif
