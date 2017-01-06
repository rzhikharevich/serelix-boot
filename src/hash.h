#ifndef SRLXBOOT_HASH_H
#define SRLXBOOT_HASH_H

#include <stddef.h>

size_t HashString(const char *str);
size_t HashBytes(const void *bytes, size_t size);

#endif
