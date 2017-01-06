#include "global.h"
#include <stdbool.h>

EFI_HANDLE gImageHandle;
EFI_SYSTEM_TABLE *gSystemTable;

void InitSystem(
	EFI_HANDLE imageHandle,
	EFI_SYSTEM_TABLE *systemTable
) {
	gImageHandle = imageHandle;
	gSystemTable = systemTable;

	extern InitFunc __start_srlx_init;
	extern InitFunc __stop_srlx_init;

	InitFunc *initializer = &__start_srlx_init;

	while (initializer < &__stop_srlx_init) {
		if ((*initializer++)() != EFI_SUCCESS) {
			gSystemTable->ConOut->OutputString(
				gSystemTable->ConOut,
				L"Initialization failed.\n\r"
			);

			// TODO: exit after a key press
			while (true) {
#if defined(__x86_64__) || defined(_M_X64)
				__asm__ __volatile__ ("pause");
#endif
			}
		}
	}
}
