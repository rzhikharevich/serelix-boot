#include "global.h"
#include "stdio.h"
#include "vfs.h"
#include "cfg.h"
#include "efi_run.h"

static void ExecEntry(Entry *entry) {
	switch (entry->imageType) {
	case ImageEFI: {
		if (entry->imagePath.size == 0) {
			printf("Missing image_path.\n");
			return;
		}

		char path[entry->imagePath.size + 1];
		memcpy(path, entry->imagePath.base, entry->imagePath.size);
		path[entry->imagePath.size] = '\0';

		VFSNode *file = NULL;
		if (VFSGetNodeAtPath(&file, path) != VFSSuccess) {
			printf("Failed to open the image.\n");
			return;
		}

		void *img;
		size_t size;
		if (VFSReadAll(file, &img, &size, false) != VFSSuccess) {
			printf("Failed to read the image.\n");
			VFSRelease(file);
			return;
		}

		VFSRelease(file);

		if (
			RunEFIImage(
				img, size,
				entry->commandLine.base,
				entry->commandLine.size
			) != EFI_SUCCESS
		) {
			printf("Failed to run the image.\n");
		}

		free(img);

		break;
	} default:
		printf("Invalid image type.\n");
		break;
	}
}

int Main(void) {
	const char *configPath = "/srlxboot/boot.cfg";

	VFSNode *configFile = NULL;
	if (VFSGetNodeAtPath(&configFile, configPath) != VFSSuccess) {
		printf("Failed to open '%s'.\n", configPath);
		return 1;
	}

	char *configBytes;
	size_t configSize;
	if (VFSReadAll(configFile, (void **)&configBytes, &configSize, true) != VFSSuccess) {
		printf("Failed to read '%s'.\n", configPath);
		VFSRelease(configFile);
		return 1;
	}

	VFSRelease(configFile);

	Config config;
	if (!ConfigInit(&config, (INIStringSlice){configBytes, configSize})) {
		free(configBytes);
		printf("Memory allocation failed.\n");
		return 1;
	}

	printf("Please choose one of the following boot options:\n");

	Entry *entries[EntryTableGetSize(&config.entries)];

	unsigned i = 0;

	for (
		EntryTableSlot *slot = EntryTableGetFirstSlot(&config.entries);
		slot; slot = EntryTableGetNextSlot(&config.entries, ++slot)
	) {
		if (i > 25) {
			printf("Too many entries! Omitting the remaining ones.\n");
			break;
		}

		printf(
			"  %c) %.*s\n",
			'a' + i,
			(int)slot->value.title.size,
			slot->value.title.base
		);

		entries[i] = &slot->value;

		i++;
	}

	(void)entries;

	printf("\n");
	
	if (config.defaultEntry) {
		unsigned t;

		for (t = config.timeout; t > 0; t--) {
			printf(
				"\rBooting the default (%.*s) in %u second%s...",
				(int)config.defaultEntry->title.size, config.defaultEntry->title.base,
				t, t > 1 ? "s" : ""
			);
		
			for (i = 0; i < 10; i++) {
				EFI_INPUT_KEY key;
			
				if (
					gSystemTable->ConIn->ReadKeyStroke(
						gSystemTable->ConIn, &key
					) == EFI_SUCCESS
				) {
					if (
						key.UnicodeChar >= L'a' && key.UnicodeChar <= L'z' &&
						(unsigned)(key.UnicodeChar - L'a') < EntryTableGetSize(&config.entries)
					) {
						ExecEntry(entries[key.UnicodeChar - L'a']);
					} else {
						goto no_timeout;
					}
				}
	
				gSystemTable->BootServices->Stall(100000);
			}
		}

		ExecEntry(config.defaultEntry);
	}

no_timeout:

	while (true) {
		UINTN eventIndex;

		gSystemTable->BootServices->WaitForEvent(
			1, &gSystemTable->ConIn->WaitForKey, &eventIndex
		);

		EFI_INPUT_KEY key;

		gSystemTable->ConIn->ReadKeyStroke(
			gSystemTable->ConIn, &key
		);

		if (
			key.UnicodeChar >= L'a' && key.UnicodeChar <= L'z' &&
			(unsigned)(key.UnicodeChar - L'a') < EntryTableGetSize(&config.entries)
		) {
			ExecEntry(entries[key.UnicodeChar - L'a']);
		}
	}
}
