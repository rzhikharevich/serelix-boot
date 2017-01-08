#include "global.h"
#include "unicode.h"
#include "stdio.h"
#include "stdlib.h"
#include "cbprintf.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static char *intcvt(
	char *dst, uintmax_t value,
	uintmax_t base, bool upper
) {
	dst += 20;

	do {
		uintmax_t r = value % base;

		*--dst = (r >= 10) ?
			(upper ? 'A' : 'a') + r - 10 :
			'0' + r;

		value /= base;
	} while (value);

	return dst;
}

int vcbprintf(cbprintf_cb print, void *ctx, const char *fmt, va_list va) {
	int n = 0;

	char buf[20];

	while (*fmt) {
		if (*fmt == '%') {
			int precision = -1;

			if (fmt[1] == '.' && fmt[2] == '*') {
				precision = va_arg(va, int);
				fmt += 2;
			}

			enum {
				LEN_DEFAULT,
				LEN_LONG,
				LEN_LLONG,
				LEN_SIZE
			} len = LEN_DEFAULT;
		
			const char *lmod = fmt + 1;

			const char *spec;

			switch (lmod[0]) {
			case 'l':
				if (lmod[1] == 'l') {
					len = LEN_LLONG;
					spec = lmod + 2;
				} else {
					len = LEN_LONG;
					spec = lmod + 1;
				}
				
				break;
			case 'z':
				len = LEN_SIZE;
				spec = lmod + 1;
				break;
			default:
				spec = lmod;
				break;
			}

			enum {T_INT, T_FLOAT, T_CHAR, T_STR} type;
			uintmax_t base;
			bool signd, upper = false;

			switch (*spec) {
			case 'i':
			case 'd':
				type = T_INT;
				base = 10;
				signd = true;
				break;
			case 'u':
				type = T_INT;
				base = 10;
				signd = false;
				break;
			case 'p':
				type = T_INT;
				base = 0x10;
				signd = false;
				len = LEN_SIZE;
				break;
			case 'x':
				type = T_INT;
				base = 0x10;
				signd = false;
				break;
			case 'X':
				type = T_INT;
				base = 0x10;
				signd = false;
				upper = true;
				break;
			case 'c':
				type = T_CHAR;
				break;
			case 's':
				type = T_STR;
				break;
			default:
				goto plain_ch;
			}

			switch (type) {
			case T_INT: {
				bool negative;
				uintmax_t value;

				if (signd && len != LEN_SIZE) {
					intmax_t svalue;

					switch (len) {
					case LEN_DEFAULT:
						svalue = va_arg(va, int); break;
					case LEN_LONG:
						svalue = va_arg(va, long); break;
					case LEN_LLONG:
						svalue = va_arg(va, long long); break;
					case LEN_SIZE:
						__builtin_unreachable();
					}

					if (svalue < 0) {
						negative = true;
						value = -svalue;
					} else {
						negative = false;
						value = (uintmax_t)svalue;
					}
				} else {
					negative = false;

					switch (len) {
					case LEN_DEFAULT:
						value = va_arg(va, unsigned int); break;
					case LEN_LONG:
						value = va_arg(va, unsigned long); break;
					case LEN_LLONG:
						value = va_arg(va, unsigned long long); break;
					case LEN_SIZE:
						value = va_arg(va, size_t); break;
					}
				}

				if (negative)
					print(ctx, "-", 1);

				char *value_str = intcvt(buf, value, base, upper);
				int value_len = 20 - (value_str - buf);

				if (!print(ctx, value_str, value_len))
					return -1;

				n += value_len;
				
				break;
			} case T_FLOAT:
				// not implemented
				exit(1);
			case T_CHAR:
				switch (len) {
				case LEN_DEFAULT: {
					char c = (char)va_arg(va, int);
					if (!print(ctx, &c, 1))
						return -1;

					break;
				} case LEN_LONG: {
					CHAR16 c16 = (CHAR16)va_arg(va, int);
					char c[4];
					if (!print(ctx, c, UTF16ToUTF8(c, &c16)))
						return -1;

					break;
				} default:
					return -1;
				}

				n++;
				
				break;
			case T_STR: {
				if (len != LEN_DEFAULT)
					return -1;
			
				char *s = va_arg(va, char *);

				size_t slen = precision > -1 ?
					(size_t)precision : strlen(s);

				if (!print(ctx, s, slen))
					return -1;

				n += (int)slen;
				
				break;
			}
			}

			fmt = spec + 1;
		} else {
		plain_ch:
			if (!print(ctx, fmt, 1))
				return -1;

			n++;
			fmt++;
		}
	}

	return n;
}

#define VA_WRAPPER(f) { \
	va_list va; \
	va_start(va, fmt); \
	int r = f; \
	va_end(va); \
	return r; \
}

int cbprintf(cbprintf_cb print, void *ctx, const char *fmt, ...) VA_WRAPPER(
	vcbprintf(print, ctx, fmt, va)
);

typedef struct {
	char *dst;
	size_t size;
} snprintf_ctx;

static bool snprintf_cb(snprintf_ctx *ctx, const char *string, size_t len) {
	if (!ctx->size)
		return true;

	size_t n = ctx->size < len ? ctx->size : len;

	memcpy(ctx->dst, string, n);

	ctx->dst += n;
	ctx->size -= n;

	return true;
}

int vsnprintf(char *restrict str, size_t size, const char *restrict fmt, va_list va) {
	snprintf_ctx ctx = {str, size};

	if (size)
		ctx.size--;

	int r = vcbprintf((cbprintf_cb)snprintf_cb, &ctx, fmt, va);

	*ctx.dst = '\0';

	return r;
}

int snprintf(char *restrict str, size_t size, const char *restrict fmt, ...) VA_WRAPPER(
	vsnprintf(str, size, fmt, va)
);

cbprintf_cb g_printf_cb;

int vprintf(const char *fmt, va_list va) {
	return vcbprintf(g_printf_cb, NULL, fmt, va);
}

int printf(const char *fmt, ...) VA_WRAPPER(
	vprintf(fmt, va)
);

setcur_cb g_setcur_cb;

bool setcur(bool value) {
	static bool lastValue = false;

	bool oldValue = lastValue;

	if (lastValue != value)
		g_setcur_cb(lastValue = value);

	return oldValue;
}

char *dgets(void) {
	bool oldcur = setcur(true);

	size_t size = 128;
	char *input = malloc(size);

	size_t i = 0;

	if (!input)
		return NULL;

	while (true) {
		if (i >= size) {
			size *= 2;
			if (!(input = reallocf(input, size)))
				return NULL;
		}

		UINTN eventIndex;

		gSystemTable->BootServices->WaitForEvent(
			1, &gSystemTable->ConIn->WaitForKey, &eventIndex
		);

		EFI_INPUT_KEY key;

		if (gSystemTable->ConIn->ReadKeyStroke(
			gSystemTable->ConIn, &key
		) != EFI_SUCCESS) {
			free(input);
			return NULL;
		}

		if (key.UnicodeChar == '\r')
			break;
		else if (key.UnicodeChar == '\b') {
			if (i) {
				i--;
				printf("\b");
			}
		} else {
			i += UTF16ToUTF8(input + i, &key.UnicodeChar);
			printf("%lc", key.UnicodeChar);
		}
	}

	input[i] = '\0';

	setcur(oldcur);

	printf("\n");

	return input;
}
