#ifndef SRLXBOOT_STDLIB_H
#define SRLXBOOT_STDLIB_H

#include <stddef.h>
#include "attr.h"

ATTR_NORET
void exit(int code);

ATTR_MALLOC
void *malloc(size_t size);
void *reallocf(void *obj, size_t size);
void free(void *obj);

#endif
