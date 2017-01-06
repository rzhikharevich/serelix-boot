#ifndef SRLXBOOT_STDARG_H
#define SRLXBOOT_STDARG_H

#define va_list __builtin_va_list

#define va_start(va, s) __builtin_va_start(va, s)
#define va_end(va)      __builtin_va_end(va)
#define va_arg(va, t)   __builtin_va_arg(va, t)
#define va_copy(d, s)   __builtin_va_copy(d, s)

int vprintf(const char *fmt, va_list va);

#endif
