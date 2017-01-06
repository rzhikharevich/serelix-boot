#ifndef SRLXBOOT_CBPRINTF_H
#define SRLXBOOT_CBPRINTF_H

#include "stdarg.h"
#include <stddef.h>
#include <stdbool.h>

typedef bool (*cbprintf_cb)(void *ctx, const char *string, size_t len);

int vcbprintf(cbprintf_cb print, void *ctx, const char *fmt, va_list va);
int cbprintf(cbprintf_cb print, void *ctx, const char *fmt, ...);

extern cbprintf_cb g_printf_cb;

#endif