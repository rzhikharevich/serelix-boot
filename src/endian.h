#ifndef SRLXBOOT_ENDIAN_H
#define SRLXBOOT_ENDIAN_H

#include <stdint.h>

#ifdef __GNUC__
// GNU
static inline uint32_t SwapBytes32(uint32_t i) {return __builtin_bswap32(i);}
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define LITTLE_ENDIAN 1
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define BIG_ENDIAN 1
#else
#error "unsupported endianness"
#endif
#elif defined(_MSC_VER)
// Microsoft
static inline uint32_t SwapBytes32(uint32_t i) {return _byteswap_ulong(i);}
// FIXME: don't assume little endian?
#define LITTLE_ENDIAN 1
#else
#error "unsupported compiler"
#endif

static inline uint32_t ShiftHigher32(uint32_t i, unsigned n) {
#if LITTLE_ENDIAN
	return i << n;
#else
	return i << (24 - n);
#endif
}

static inline uint32_t FromLittle32(uint32_t i) {
#if LITTLE_ENDIAN
	return i;
#else
	return SwapBytes32(i);
#endif
}

static inline uint32_t FromBig32(uint32_t i) {
#if LITTLE_ENDIAN
	return SwapBytes32(i);
#else
	return i;
#endif
}

#endif
