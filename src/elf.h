#ifndef SRLXBOOT_ELF_H
#define SRLXBOOT_ELF_H

#include <stdint.h>
#include <stdbool.h>
#include "attr.h"
#include "endian.h"

typedef struct {
	uint32_t magic;
	uint8_t bits;
	uint8_t end;
	uint8_t version1;
	uint8_t abi;
	uint8_t abiver;
	uint8_t __padding[7];
	uint16_t type;
	uint16_t machine;
	uint32_t version2;
	uint64_t entry;
	uint64_t phoff;
	uint64_t shoff;
	uint32_t flags;
	uint16_t size;
	uint16_t phentsize;
	uint16_t phnum;
	uint16_t shentsize;
	uint16_t shnum;
	uint16_t shstrndx;
} ATTR_PACKED ELFHeader;

#define ELFMagic FromBig32(0x7F454C46)

enum {
	ELFBits32 = 1,
	ELFBits64
};

enum {
	ELFEndLittle = 1,
	ELFEndBig
};

enum {
	ELFVersionCurrent = 1
};

enum {
	ELFABISystemV = 0
};

enum {
	ELFTypeNone = 0,
	ELFTypeRel  = 1,
	ELFTypeExec = 2,
	ELFTypeDyn  = 3,
	ELFTypeCore = 4
};

enum {
	ELFMachineX86_64 = 0x3E
};

typedef struct {
	uint32_t type;
	uint32_t flags;
	uint64_t offset;
	uint64_t vaddr;
	uint64_t paddr;
	uint64_t fileSize;
	uint64_t memSize;
	uint64_t align;
} ELFProgramHeader;

enum {
	ELFProgramLoad = 1
};

enum {
	ELFProgramX = 1,
	ELFProgramW = 2,
	ELFProgramR = 4
};

bool ELFHeaderCheck(ELFHeader *hdr);

#endif
