#include "elf.h"
#include "stdio.h"

bool ELFHeaderCheck(ELFHeader *hdr) {
	if (hdr->magic != ELFMagic) {
		printf("Bad ELF magic.\n");
		return false;
	}

	if (
		hdr->bits    != ELFBits64 ||
		hdr->end     != ELFEndLittle ||
		hdr->machine != ELFMachineX86_64
	) {
		printf("Bad ELF arch.\n");
		return false;
	}

	if (
		hdr->version1 != ELFVersionCurrent ||
		hdr->version2 != ELFVersionCurrent
	) {
		printf("Bad ELF version.\n");
		return false;
	}

	if (hdr->abi != ELFABISystemV) {
		printf("Bad ELF ABI.\n");
		return false;
	}

	if (hdr->type != ELFTypeDyn) {
		printf("Bad ELF type.\n");
		return false;
	}

	return true;
}
