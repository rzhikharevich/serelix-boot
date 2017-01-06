#include <string.h>

char *strcpy(char *__restrict__ dest, const char *__restrict__ src)
{
	const unsigned char *s = src;
	unsigned char *d = dest;
	while ((*d++ = *s++));
	return dest;
}
