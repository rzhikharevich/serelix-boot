#ifndef SRLXBOOT_STDIO_H
#define SRLXBOOT_STDIO_H

#include <stddef.h>
#include <stdbool.h>
#include "attr.h"

ATTR_FMT(gnu_printf, 3, 4)
int snprintf(char *restrict str, size_t size, const char *restrict fmt, ...);

ATTR_FMT(gnu_printf, 1, 2)
int printf(const char *restrict fmt, ...);

typedef void (*setcur_cb)(bool value);
extern setcur_cb g_setcur_cb;
bool setcur(bool value);

ATTR_MALLOC
char *dgets(void);

#endif
