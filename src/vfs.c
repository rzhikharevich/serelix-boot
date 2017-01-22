#include "global.h"
#include "vfs.h"
#include "stdlib.h"
#include <string.h>
#include <stdbool.h>
#include "string.h"

static VFSNode *root = NULL;

VFSReturn VFSMountRoot(VFSNode *node) {
	if (root)
		return VFSInvalidArg;

	VFSRetain(node);
	root = node;

	return VFSSuccess;
}

void VFSShrinkPath(char *dst, const char *src) {
	bool absolute = *src == '/';

	char *dstStart = dst;

	while (*src) {
		if (
			!strncmp(src, "/..", 3) && (
				src[3] == '\0' || src[3] == '/'
			)
		) {
			if (dst == dstStart || !strcmp(dst - 3, "../")) {
				strcpy(dst, "../");
				dst += 3;
				src += 3;
			} else {
				dst = memrchr(dstStart, '/', dst - dstStart);
				if (!dst)
					dst = dstStart;

				dst[1] = 0;

				src += 3;
			}
		} else if (
			!strncmp(src, "/.", 2) && (
				src[2] == '\0' || src[2] == '/'
			)
		)
			src += 2;
		else if (!strncmp(src, "//", 2))
			src++;
		else
			*dst++ = *src++;
	}

	*dst = '\0';

	if (absolute)
		*dstStart = '/';
	else if (*dstStart == '\0' || *dstStart == '/')
		*dstStart = '.';
}

VFSReturn VFSGetNodeAtPath(VFSNode **node, const char *path) {
	char shpath[strlen(path) + 1];
	VFSShrinkPath(shpath, path);

	char *p = shpath;

	if (*p == '/') {
		VFSRetain(root);
		VFSRelease(*node);
		*node = root;
		p++;
	} else {
		if (!*node)
			return VFSInvalidArg;

		while (!strcmp(p, "../")) {
			p += 3;
			if ((*node)->parent) {
				VFSRetain((*node)->parent);
				VFSNode *parent = (*node)->parent;
				VFSRelease(*node);
				*node = parent;
			}
		}
	}

	while (*p) {
		char *end = p;
		while (*end != '\0' && *end != '/')
			end++;

		size_t compLen = end - p;
		if (compLen == 0) {
			p++;
			continue;
		}

		char comp[compLen + 1];
		memcpy(comp, p, compLen);
		comp[compLen] = '\0';

		VFSReturn ret = VFSGetChild(*node, node, comp);
		if (ret != VFSSuccess) {
			return ret;
		}

		p = end;
	}

	return VFSSuccess;
}

VFSReturn VFSGetChild(VFSNode *node, VFSNode **child, const char *name) {
	if (!(node->attributes & VFSNodeDirectory))
		return VFSNotFound;

	VFSDirNode *dir = (VFSDirNode *)node;

	VFSNode **childp = VFSCacheGet(&dir->cache, &name);

	if (childp) {
		VFSRetain(*child = *childp);
		return VFSSuccess;
	}

	VFSReturn ret = node->ops->getChild(node, child, name);
	if (ret != VFSSuccess)
		return ret;

	if (
		//(*child)->attributes & VFSNodeDirectory &&
		!VFSCacheInsert(&dir->cache, &name, child)
	) {
		VFSRelease(*child);
		return VFSFailure;
	}

	// TODO: resolve ref cycle.
	VFSRetain(*child); // cached reference

	return VFSSuccess;
}

VFSReturn VFSReadAll(VFSNode *node, void **contents, size_t *size, bool terminate) {
	VFSReturn ret;

	VFSOffset realSize;
	if ((ret = VFSGetSize(node, &realSize)) != VFSSuccess)
		return ret;

#if SIZE_MAX < UINT64_MAX
	if ((realSize >> 32))
		return VFSNoMemory;
#endif

	if (size)
		*size = (size_t)realSize;

	size_t allocSize = (size_t)realSize;
	if (terminate)
		allocSize++;

	if (!(*contents = malloc(allocSize)))
		return VFSNoMemory;

	if ((ret = VFSRead(node, *contents, 0, (size_t)realSize)) != VFSSuccess)
		return ret;

	if (terminate)
		((char *)*contents)[allocSize - 1] = '\0';

	return VFSSuccess;
}

const char *VFSReturnToString(VFSReturn ret) {
	static const char *strings[] = {
		"success",
		"failure",
		"is a directory",
		"is not a directory",
		"not found",
		"invalid argument",
		"operation not supported",
		"no memory"
	};

	return (ret >= sizeof(strings) / sizeof(strings[0])) ?
		"undefined error" : strings[ret];
}
