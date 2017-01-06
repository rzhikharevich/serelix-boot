#ifndef SRLXBOOT_EFI_RUN_H
#define SRLXBOOT_EFI_RUN_H

#include <Uefi.h>
#include <stddef.h>

EFI_STATUS RunEFIImage(void *exec, size_t execSize, const char *params, size_t paramsSize);

#endif
