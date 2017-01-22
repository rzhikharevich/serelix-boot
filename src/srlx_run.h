#ifndef SRLXBOOT_SRLX_RUN_H
#define SRLXBOOT_SRLX_RUN_H

#include <stddef.h>

void RunSerelixImage(void *exec, size_t execSize, const char *params, size_t paramsSize);

#endif
