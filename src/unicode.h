#ifndef SRLXBOOT_UTF8_H
#define SRLXBOOT_UTF8_H

#include <Base.h>
#include <stdint.h>
#include "intrinsics.h"

typedef uint32_t UnicodeScalar;

static const unsigned UTF8InvalidLength = 0;

static inline unsigned UTF8GetCharLength(char ch) {
	if (ch == 0)
		return UTF8InvalidLength;

	int lz = CountLeadingZeros32(~((unsigned)ch << 24));

	if (lz > 4 || lz == 1)
		return UTF8InvalidLength;

	if (lz == 0)
		return 1;

	return (unsigned)lz;
}

static inline UnicodeScalar UTF8ToScalar(const char *ch) {
	unsigned len = UTF8GetCharLength(*ch);

	UnicodeScalar r;

	const char cont = 0x3F;

	const unsigned char *uch = (const unsigned char *)ch;

	switch (len) {
	case 1 : r = (UnicodeScalar) uch[0]; break;
	case 2 : r = (UnicodeScalar)(uch[0] & 0x1F <<  6) | (uch[1] & cont); break;
	case 3 : r = (UnicodeScalar)(uch[0] & 0x0F << 12) | (uch[1] & cont <<  6) | (uch[2] & cont); break;
	case 4 : r = (UnicodeScalar)(uch[0] & 0x07 << 18) | (uch[1] & cont << 12) | (uch[2] & cont << 6) | (uch[3] & cont); break;
	default: r = 0; break;
	}

	return r;
}

static inline size_t UTF8ToUTF16(CHAR16 *utf16, const char *utf8) {
	UnicodeScalar scalar = UTF8ToScalar(utf8);
	if (scalar > 0xFFFF) {
		// TODO: check correctness.
		scalar -= 0x10000;
		utf16[0] = scalar >> 10;
		utf16[1] = scalar & ((1 << 10) - 1);
		return 2;
	} else {
		*utf16 = scalar;
		return 1;
	}
}

static inline CHAR16 *UTF8ToUTF16String(CHAR16 *utf16, const char *utf8) {
	while (*utf8) {
		utf16 += UTF8ToUTF16(utf16, utf8);
		utf8  += UTF8GetCharLength(*utf8);
	}

	*utf16 = L'\0';

	return utf16;
}

static inline CHAR16 *SizedUTF8ToUTF16String(CHAR16 *utf16, const char *utf8, size_t size) {
	for (const char *utf8End = utf8 + size; utf8 < utf8End && *utf8;) {
		utf16 += UTF8ToUTF16(utf16, utf8);
		utf8 += UTF8GetCharLength(*utf8);
	}

	*utf16 = L'\0';

	return utf16;
}

// TODO: surrogate pairs
static inline UnicodeScalar UTF16ToScalar(const CHAR16 *utf16) {
	return utf16[0];
}

// TODO: surrogate pairs
static inline size_t UTF16ToUTF8(char *utf8, const CHAR16 *utf16) {
	UnicodeScalar scalar = UTF16ToScalar(utf16);

	if (scalar <= 0x7F) {
		utf8[0] = (char)scalar;
		return 1;
	} else if (scalar <= 0x7FF) {
		utf8[0] = (char)(scalar >> 6);
		utf8[1] = (char)(scalar & 0x3F);
		return 2;
	} else if (scalar <= 0xFFFF) {
		utf8[0] = (char)(scalar >> 12);
		utf8[1] = (char)((scalar >> 6) & 0x3F);
		utf8[2] = (char)(scalar & 0x3F);
		return 3;
	} else {
		UTF16ToUTF8(utf8, L"\xFFFD");
		return 3;
	}
}

#endif
