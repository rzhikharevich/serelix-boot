#include "global.h"
#include "vfs.h"
#include "unicode.h"
#include "stdlib.h"
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>

static VFSReturn EFIGetChild(VFSNode *node, VFSNode **child, const char *name);
static VFSReturn EFIGetSize(VFSNode *node, VFSOffset *size);
static VFSReturn EFIRead(VFSNode *node, void *contents, VFSOffset offset, size_t size);
static void EFIDestroy(VFSNode *node);

static VFSOpTable ops = {
	.getChild = EFIGetChild,
	.getSize = EFIGetSize,
	.read = EFIRead,
	.destroy = EFIDestroy
};

typedef struct {
	VFSNode node;
	EFI_FILE *file;
} VFSNodeEFI;

typedef struct {
	VFSDirNode dirNode;
	EFI_FILE *file;
} VFSDirNodeEFI;

static VFSDirNodeEFI root = {
	.dirNode = {
		.fileNode = {
			.ops        = &ops,
			.parent     = NULL,
			.refCount   = 1,
			.attributes = VFSNodeDirectory
		}
	},

	.file = NULL
};

static EFI_GUID fileInfoId = EFI_FILE_INFO_ID;

static size_t getInfoSize(EFI_FILE *file) {
	UINTN infoSize = 0;
	if (file->GetInfo(file, &fileInfoId, &infoSize, NULL) != EFI_BUFFER_TOO_SMALL)
		return 1;

	return (size_t)infoSize;
}

static bool getInfo(EFI_FILE *file, EFI_FILE_INFO *info, size_t size) {
	return
		size > 1 &&
		file->GetInfo(file, &fileInfoId, (UINTN *)&size, info) == EFI_SUCCESS;
}

static EFI_FILE **getFile(VFSNode *node) {
	if (node->attributes & VFSNodeDirectory)
		return &((VFSDirNodeEFI *)node)->file;
	else
		return &((VFSNodeEFI *)node)->file;
}

static VFSReturn EFIGetChild(VFSNode *node, VFSNode **child, const char *name) {
	EFI_FILE *file = *getFile(node);

	CHAR16 name16[strlen(name) + 1];
	UTF8ToUTF16String(name16, name);

	// open child file

	EFI_FILE *childFile;
	if (file->Open(file, &childFile, name16, EFI_FILE_MODE_READ, 0) != EFI_SUCCESS)
		return VFSFailure;

	// parse attributes

	VFSNodeAttributes attributes = 0;

	size_t infoSize = getInfoSize(childFile);
	
	union {
		EFI_FILE_INFO ty;
		char bytes[infoSize];
	} info;

	if (
		getInfo(childFile, &info.ty, infoSize) &&
		info.ty.Attribute & EFI_FILE_DIRECTORY
	)
		attributes |= VFSNodeDirectory;

	// allocate child node

	if (!(*child = malloc(
		attributes & VFSNodeDirectory ?
			sizeof(VFSDirNodeEFI) :
			sizeof(VFSNodeEFI)
	))) {
		childFile->Close(childFile);
		return VFSNoMemory;
	}

	// set file node properties

	(*child)->ops = &ops;
	VFSRetain(node);
	(*child)->parent = node;
	(*child)->refCount = 1;
	(*child)->attributes = attributes;
	*getFile(*child) = childFile;

	if (attributes & VFSNodeDirectory) {
		VFSCache *cache = &((VFSDirNode *)*child)->cache;
		if (!VFSCacheInit(cache))
			VFSCacheInitWithCapacity(cache, 0);
	}

	return VFSSuccess;
}

static VFSReturn EFIGetSize(VFSNode *node, VFSOffset *size) {
	EFI_FILE *file = ((VFSNodeEFI *)node)->file;

	size_t infoSize = getInfoSize(file);

	union {
		EFI_FILE_INFO ty;
		char bytes[infoSize];
	} info;

	if (!getInfo(file, &info.ty, infoSize))
		return VFSFailure;

	*size = info.ty.FileSize;

	return VFSSuccess;
}

static VFSReturn EFIRead(VFSNode *node, void *contents, VFSOffset offset, size_t size) {
	EFI_FILE *file = ((VFSNodeEFI *)node)->file;

	if (file->SetPosition(file, offset) != EFI_SUCCESS)
		return VFSFailure;

	if (file->Read(file, (UINTN *)&size, contents) != EFI_SUCCESS)
		return VFSFailure;

	return VFSSuccess;
}

static void EFIDestroy(VFSNode *node) {
	EFI_FILE *file = ((VFSNodeEFI *)node)->file;
	file->Close(file);
}

static EFI_STATUS RootInit(void) {
	EFI_STATUS status;

	/*
	 * Get the loaded image protocol.
	 */

	EFI_GUID loadedImageProtoId = EFI_LOADED_IMAGE_PROTOCOL_GUID;

	EFI_LOADED_IMAGE_PROTOCOL *loadedImage;

	status = gSystemTable->BootServices->HandleProtocol(
		gImageHandle, &loadedImageProtoId,
		(void **)&loadedImage
	);

	if (status != EFI_SUCCESS)
		return status;

	/*
	 * Get the simple file system protocol for the boot volume.
	 */

	EFI_GUID fsProtoId = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *bootVolume;

	status = gSystemTable->BootServices->HandleProtocol(
		loadedImage->DeviceHandle, &fsProtoId,
		(void **)&bootVolume
	);

	if (status != EFI_SUCCESS)
		return status;

	/*
	 * Mount the boot volume as the VFS root.
	 */

	status = bootVolume->OpenVolume(bootVolume, &root.file);

	if (status != EFI_SUCCESS)
		return status;

	VFSCacheInit(&root.dirNode.cache);

	VFSMountRoot(&root.dirNode.fileNode);

	return EFI_SUCCESS;
}

INIT_FUNC(RootInit);
