#include "cfg.h"
#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include <limits.h>

static unsigned INIStringSliceToUIntDec(INIStringSlice slice, bool *ok) {
	if (slice.size > 10) {
		*ok = false;
		return 0;
	}

	size_t x = 0;

	*ok = true;

	for (size_t i = 0; i < slice.size; i++) {
		unsigned d = slice.base[i] - '0';

		if (
			slice.base[i] > '9' || slice.base[i] < '0' ||
			x > UINT_MAX / 10 || x * 10 > UINT_MAX - d
		) {
			*ok = false;
			return 0;
		}

		x = x * 10 + d;
	}

	return x;
}

static void HandleConfigEntry(
	void *context,
	INIStringSlice section,
	INIStringSlice key,
	INIStringSlice value
) {
	Config *cfg = context;

	if (!strncmp(section.base, "common", section.size)) {
		if (INIStringSliceEqStr(key, "default")) {
			cfg->defaultEntryKey = value;
		} else if (INIStringSliceEqStr(key, "timeout")) {
			bool ok;
			unsigned t = INIStringSliceToUIntDec(value, &ok);

			if (ok)
				cfg->timeout = t;
			else
				printf("common.timeout must be a decimal number.\n");
		}
	} else {
		Entry *entry = EntryTableGet(&cfg->entries, &section);

		if (!entry) {
			Entry def = {
				.title = section,
				.imageType = ImageEFI,
				.imagePath = {0, 0},
				.commandLine = {0, 0}
			};

			if (!(entry = EntryTableInsert(&cfg->entries, &section, &def))) {
				printf("Memory allocation failed.\n");
				exit(1);
			}
		}

#define PROP(k, v) \
	if (INIStringSliceEqStr(key, k)) \
		entry->v = value;

		PROP("title", title)
		else if (INIStringSliceEqStr(key, "image_type")) {

#define IMGTY(s, v) \
	if (INIStringSliceEqStr(value, s)) \
		entry->imageType = v;

			IMGTY("efi", ImageEFI)
			else {
				cfg->ok = false;
				
				printf(
					"Unknown image type: %.*s\n",
					(int)value.size, value.base
				);

				entry->imageType = ImageInvalid;
			}

#undef IMGTY

		} else PROP("image_path", imagePath)
		else PROP("cmdline", commandLine)
		else {
			cfg->ok = false;

			printf(
				"Unknown entry property: %.*s\n",
				(int)value.size, value.base
			);
		}

#undef PROP
	}
}

static void HandleConfigError(void *context, INILocation location, const char *msg) {
	(void)context;

	printf("boot.cfg:%u:%u: %s\n", location.line, location.column, msg);
}

bool ConfigInit(Config *self, INIStringSlice source) {
	self->ok = true;
	self->defaultEntryKey = (INIStringSlice){NULL, 0};
	self->defaultEntry = NULL;
	self->timeout = 0;

	if (!EntryTableInit(&self->entries))
		return false;

	bool configOk = INIParse(
		source,
		self, HandleConfigEntry,
		NULL, HandleConfigError
	);

	if (self->ok)
		self->ok = configOk;

	if (self->defaultEntryKey.size > 0)
		self->defaultEntry = EntryTableGet(&self->entries, &self->defaultEntryKey);

	return true;
}
