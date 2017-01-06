#include "efi_run.h"
#include "global.h"
#include "unicode.h"
#include <Protocol/LoadedImage.h>
#include "stdlib.h"
#include "stdio.h"
#include <string.h>

EFI_STATUS RunEFIImage(void *exec, size_t execSize, const char *params, size_t paramsSize) {
	EFI_STATUS ret;

	EFI_HANDLE execHandle;

	ret = gSystemTable->BootServices->LoadImage(
		TRUE, gImageHandle,
		NULL, exec, execSize,
		&execHandle
	);

	if (ret != EFI_SUCCESS)
		return ret;

	EFI_GUID loadedImageProtoID = EFI_LOADED_IMAGE_PROTOCOL_GUID;

	EFI_LOADED_IMAGE *execLoadedImage;

	// TODO: somehow release execLoadedImage?
	ret = gSystemTable->BootServices->HandleProtocol(
		execHandle, &loadedImageProtoID, (void **)&execLoadedImage
	);

	if (ret != EFI_SUCCESS)
		goto cleanup;

	CHAR16 *params16 = malloc((paramsSize + 1) * sizeof(CHAR16));
	CHAR16 *params16End = SizedUTF8ToUTF16String(params16, params, paramsSize);

	execLoadedImage->LoadOptions = params16;
	execLoadedImage->LoadOptionsSize = (uintptr_t)params16End - (uintptr_t)params16;

	ret = gSystemTable->BootServices->StartImage(execHandle, NULL, NULL);

cleanup:
	gSystemTable->BootServices->UnloadImage(execHandle);

	return ret;
}
