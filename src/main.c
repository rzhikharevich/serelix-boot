#include "global.h"
#include "stdio.h"
#include "vfs.h"
#include "efi_run.h"

int Main(void) {
	printf("Loading kernel... ");

	VFSNode *kernelFile = NULL;
	if (VFSGetNodeAtPath(&kernelFile, "/linux.efi") != VFSSuccess) {
		printf("failed!\n");
		return 1;
	}

	char *kernel;
	size_t kernelSize;
	if (VFSReadAll(kernelFile, (void **)&kernel, &kernelSize, true) != VFSSuccess) {
		printf("failed!\n");
		return 1;
	}

	printf(
		"done (%zu bytes at 0x%p).\n"
		"Transferring control to the kernel.\n",
		kernelSize, kernel
	);

	RunEFIImage(kernel, kernelSize, "rootwait", 8);

	return 0;
}
