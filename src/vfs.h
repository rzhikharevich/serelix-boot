#ifndef SRLXBOOT_VFS_H
#define SRLXBOOT_VFS_H

#include "stdlib.h"
#include <stdint.h>
#include <stdbool.h>
#include "hashtab.h"

typedef enum {
	VFSSuccess,
	VFSFailure,
	VFSIsDir,
	VFSIsNotDir,
	VFSNotFound,
	VFSInvalidArg,
	VFSOpNotSupported,
	VFSNoMemory
} VFSReturn;

const char *VFSReturnToString(VFSReturn ret);

typedef uint64_t VFSOffset;

typedef struct _VFSNode VFSNode;

typedef struct {
	VFSReturn (*getChild)(VFSNode *node, VFSNode **child, const char *name);
	VFSReturn (*getSize)(VFSNode *node, VFSOffset *size);
	VFSReturn (*read)(VFSNode *node, void *contents, VFSOffset offset, size_t size);
	void (*destroy)(VFSNode *node);
} VFSOpTable;

typedef enum {
	VFSNodeDirectory = 1 << 0
} VFSNodeAttributes;

HashTabType(const char *, VFSNode *, StringKeyHash, StringKeyEq, VFSCache);

struct _VFSNode {
	VFSOpTable *ops;
	VFSNode *parent;
	size_t refCount;
	VFSNodeAttributes attributes;
};

typedef struct {
	VFSNode fileNode;
	VFSCache cache;
} VFSDirNode;

VFSReturn VFSMountRoot(VFSNode *node);

void VFSShrinkPath(char *dst, const char *src);

VFSReturn VFSGetNodeAtPath(VFSNode **node, const char *path);

VFSReturn VFSGetChild(VFSNode *node, VFSNode **child, const char *name);

static inline VFSReturn VFSGetSize(VFSNode *node, VFSOffset *size) {
	return node->ops->getSize(node, size);
}

static inline VFSReturn VFSRead(VFSNode *node, void *contents, VFSOffset offset, size_t size) {
	return node->ops->read(node, contents, offset, size);
}

VFSReturn VFSReadAll(VFSNode *node, void **contents, size_t *size, bool terminate);

static inline void VFSRetain(VFSNode *node) {
	node->refCount++;
}

static inline void VFSRelease(VFSNode *node) {
	if (!node)
		return;

	if (node->refCount-- <= 1) {
		node->ops->destroy(node);
		VFSRelease(node->parent);
		free(node);
	}
}

#endif
