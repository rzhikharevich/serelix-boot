#ifndef SRLXBOOT_ATTR_H
#define SRLXBOOT_ATTR_H

#ifndef __GNUC__
#error "unsupported compiler"
#endif

#define ATTR_PACKED __attribute__((packed))

#define ATTR_FMT(type, fmtndx, argndx) __attribute__((format(type, fmtndx, argndx)))

#define ATTR_NORET __attribute__((noreturn))

#define ATTR_NONNULL_ARG(...) __attribute__((nonnull(## __VA_ARGS__)))
#define ATTR_NONNULL_RET __attribute__((returns_nonnull))

#define ATTR_MALLOC __attribute__((malloc))

#endif
