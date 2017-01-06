#include "global.h"
#include "stdio.h"
#include "cbprintf.h"
#include "unicode.h"

static bool efi_printf_cb(void *ctx, const char *string, size_t len) {
	(void)ctx;

	for (size_t i = 0; i < len; i += UTF8GetCharLength(string[i])) {
		if (string[i] == '\n') {
			gSystemTable->ConOut->OutputString(
				gSystemTable->ConOut, L"\n\r"
			);
		} else {
			CHAR16 c[3];

			if (UTF8ToUTF16(c, string + i) == 2)
				c[2] = L'\0';
			else
				c[1] = L'\0';

			gSystemTable->ConOut->OutputString(
				gSystemTable->ConOut, c
			);
		}
	}

	return true;
}

static void efi_setcur_cb(bool value) {
	gSystemTable->ConOut->EnableCursor(
		gSystemTable->ConOut, value
	);
}

static EFI_STATUS EFIConInit(void) {
	gSystemTable->ConOut->Reset(
		gSystemTable->ConOut, TRUE
	);

	gSystemTable->ConOut->EnableCursor(
		gSystemTable->ConOut, FALSE
	);

	g_printf_cb = efi_printf_cb;

	g_setcur_cb = efi_setcur_cb;

	gSystemTable->ConIn->Reset(
		gSystemTable->ConIn, TRUE
	);

	return EFI_SUCCESS;
}

INIT_FUNC(EFIConInit);
