#ifndef GLOBAL_H
#define GLOBAL_H

#include <Uefi.h>

extern EFI_SYSTEM_TABLE *gSystemTable;
extern EFI_HANDLE gImageHandle;

typedef EFI_STATUS (*InitFunc)(void);

#define INIT_FUNC(f) __attribute__((section("srlx_init"))) InitFunc _##f##init_ptr = f;

#endif
