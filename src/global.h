#ifndef GLOBAL_H
#define GLOBAL_H

#include <Uefi.h>

extern EFI_SYSTEM_TABLE *gSystemTable;
extern EFI_HANDLE gImageHandle;

typedef EFI_STATUS (*InitFunc)(void);

#ifdef _WIN32
#define INIT_FUNC(f) __attribute__((section("srlx_init"))) InitFunc _##f##init_ptr = f;
#else
// quick fix for clang-complete
#define INIT_FUNC(f)
#endif

#endif
