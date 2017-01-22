#include "global.h"
#include "srlx_run.h"
#include "stdio.h"
#include "string.h"
#include <stdbool.h>
#include "elf.h"

typedef enum {
	SXMemConventional,
	SXMemBootData,
	SXMemUnusable
} SXMemType;

typedef struct {
	SXMemType type;
	uintptr_t base;
	size_t size;
} SXMemRegion;

typedef struct {
	
} SXBootData;

typedef void (*SXEntry)(void);

void RunSerelixImage(void *exec, size_t execSize, const char *params, size_t paramsSize) {
#define PHDR(i) ((ELFProgramHeader *)((uintptr_t)exec + hdr->phoff + i * hdr->phentsize))

	(void)execSize;
	(void)params;
	(void)paramsSize;

	ELFHeader *hdr = exec;

	if (!ELFHeaderCheck(hdr))
		return;

	/*
	 * Get the memory map.
	 */

	UINTN mmapSize = 0;
	UINTN mapKey;
	UINTN mmapEntSize;
	UINT32 mmapVer;

	if (
		gSystemTable->BootServices->GetMemoryMap(
			&mmapSize,
			NULL,
			&mapKey,
			&mmapEntSize,
			&mmapVer
		) != EFI_BUFFER_TOO_SMALL
	) {
		printf("Failed to get the memory map.\n");
		return;
	}

	char mmap[mmapSize];

	if (
		gSystemTable->BootServices->GetMemoryMap(
			&mmapSize,
			(EFI_MEMORY_DESCRIPTOR *)mmap,
			&mapKey,
			&mmapEntSize,
			&mmapVer
		) != EFI_SUCCESS
	) {
		printf("Failed to get the memory map.\n");
		return;
	}

	/*
	 * Compute the kernel memory size.
	 */

	size_t kmsize = 0;

	for (size_t i = 0; i < hdr->phnum; i++) {
		ELFProgramHeader *ph = PHDR(i);

		uintptr_t end = ph->vaddr + ph->memSize;

		if (ph->type == ELFProgramLoad && end > kmsize)
			kmsize = end;
	}

	/*
	 * Process the memory map and allocate kernel memory.
	 */

	size_t mmapEntN = mmapSize / mmapEntSize;

	SXMemRegion kmmap[mmapEntN];
	SXMemRegion kseg[3];

	kseg[0].base = 1; // invalid address, i.e. kseg is not populated

	uintptr_t kbase;

	for (size_t i = 0; i < mmapEntN; i++) {
		EFI_MEMORY_DESCRIPTOR *md = (EFI_MEMORY_DESCRIPTOR *)(mmap + i * mmapEntSize);

		/*
		 * Convert the current EFI memory descriptor to the kernel's format.
		 */

		switch (md->Type) {
		case EfiLoaderCode:
		case EfiBootServicesCode:
		case EfiBootServicesData:
		case EfiRuntimeServicesCode:
		case EfiRuntimeServicesData:
		case EfiConventionalMemory:
			kmmap[i].type = SXMemConventional;
			break;
		// TODO: other types (ACPI stuff).
		default:
			kmmap[i].type = SXMemUnusable;
			break;
		}

		kmmap[i].base = md->PhysicalStart;
		kmmap[i].size = md->NumberOfPages * 4096;

		if (
			kseg[0].base == 1 &&
			kmmap[i].type == SXMemConventional &&
			kmsize <= kmmap[i].size
		) {
			/*
			 * We can fit the kernel image into the current memory region.
			 */

			kbase = kmmap[i].base;

			size_t ksegndx = 0;

			for (size_t j = 0; j < hdr->phnum; j++) {
				ELFProgramHeader *ph = PHDR(j);

				if (ph->type == ELFProgramLoad) {
					kseg[ksegndx].type = kmmap[i].type;
					kseg[ksegndx].base = kmmap[i].base + ph->vaddr;
					kseg[ksegndx].size = ph->memSize;

					memcpy(
						(void *)kseg[ksegndx].base,
						(char *)exec + ph->offset,
						ph->fileSize
					);
		
					ksegndx++;
				}
			}
		}
	}

	gSystemTable->BootServices->ExitBootServices(
		gImageHandle,
		mapKey
	);

	SXEntry kjump = (SXEntry)(kbase + hdr->entry);
	kjump();

#undef PHDR
}
