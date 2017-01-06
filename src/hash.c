#include "hash.h"
#include "intrinsics.h"

/*
 * This is an implementation of the FNV1a hash function.
 */

#if SIZE_WIDTH == 64
#define BASE  14695981039346656037ULL
#define PRIME 1099511628211
#else
#define BASE  2166136261UL
#define PRIME 16777619
#endif

size_t HashString(const char *str) {
	size_t hash = BASE;

	while (*str) {
		hash ^= *str;
		hash *= PRIME;
		str++;
	}

	return hash;
}

size_t HashBytes(const void *bytes, size_t size) {
	size_t hash = BASE;

	const char *p = bytes;

	for (size_t i = 0; i < size; i++) {
		hash ^= *p;
		hash *= PRIME;
		p++;
	}

	return hash;
}
