#include "global.h"
#include "stdio.h"
#include <stdlib.h>
#include <string.h>

void exit(int code) {
	printf(
		"exit(%d)\n"
		"Press any key to continue...\n",
		code
	);

	UINTN eventIndex;

	gSystemTable->BootServices->WaitForEvent(
		1, &gSystemTable->ConIn->WaitForKey, &eventIndex
	);

	EFI_INPUT_KEY key;

	gSystemTable->ConIn->ReadKeyStroke(
		gSystemTable->ConIn, &key
	);

	gSystemTable->BootServices->Exit(
		gImageHandle, EFI_SUCCESS, 0, NULL
	);

	printf("Failed to exit. Stop.");

	for (;;);
}

typedef struct {
	size_t size;
	char obj[];
} memblk;

void *malloc(size_t size) {
	if (!size)
		return NULL;

	memblk *blk;

	EFI_STATUS status = gSystemTable->BootServices->AllocatePool(
		EfiLoaderData, sizeof(memblk) + size, (void **)&blk
	);

	if (status != EFI_SUCCESS)
		return NULL;

	return blk->obj;
}

void *realloc(void *obj, size_t size) {
	if (!obj)
		return malloc(size);

	if (size == 0) {
		free(obj);
		return NULL;
	}

	void *newObj = malloc(size);

	if (!newObj)
		return NULL;

	memblk *blk = (memblk *)obj - 1;

	memcpy(
		newObj, obj,
		size < blk->size ? size : blk->size
	);

	free(obj);

	return newObj;
}

void *reallocf(void *obj, size_t size) {
	void *newObj = realloc(obj, size);

	if (!newObj && size > 0)
		free(obj);

	return newObj;
}

void free(void *obj) {
	if (!obj)
		return;

	memblk *blk = (memblk *)obj - 1;

	gSystemTable->BootServices->FreePool(blk);
}

EFIAPI
EFI_STATUS Init(
	EFI_HANDLE        imageHandle,
	EFI_SYSTEM_TABLE *systemTable
) {
	void InitSystem(
		EFI_HANDLE imageHandle,
		EFI_SYSTEM_TABLE *systemTable
	);

	InitSystem(imageHandle, systemTable);

	int Main(void);

	exit(Main());
}
