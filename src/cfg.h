#ifndef SRLXBOOT_CFG_H
#define SRLXBOOT_CFG_H

#include "hashtab.h"
#include <ini.h>

typedef enum {
	ImageEFI,
	ImageSerelix,
	ImageInvalid
} ImageType;

typedef struct {
	INIStringSlice title;

	ImageType imageType;
	INIStringSlice imagePath;

	INIStringSlice commandLine;
} Entry;

static inline bool INIStringSliceEqStr(INIStringSlice slice, const char *str) {
	return strlen(str) == slice.size && !memcmp(slice.base, str, slice.size);
}

static inline size_t INIStringSliceKeyHash(INIStringSlice *slice) {
	return HashBytes(slice->base, slice->size);
}

static inline bool INIStringSliceKeyEq(INIStringSlice *a, INIStringSlice *b) {
	return a->size == b->size && !memcmp(a->base, b->base, a->size);
}

HashTabType(
	INIStringSlice, Entry,
	INIStringSliceKeyHash,
	INIStringSliceKeyEq,
	EntryTable
);

typedef struct {
	bool ok;

	INIStringSlice defaultEntryKey;
	Entry *defaultEntry;
	unsigned timeout;
	
	EntryTable entries;
} Config;

bool ConfigInit(Config *self, INIStringSlice source);

#endif
