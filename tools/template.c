#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>

#if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
# define UNIX
# include <sys/time.h>
# include <sys/param.h>
# include <fcntl.h>
# ifdef __linux__
#  include <linux/version.h>
#  if LINUX_VERSION_CODE >= KERNEL_VERSION(3,17,0) && !PORTABLE
#   include <linux/random.h>
#   define LINUX_GETRANDOM
#  endif
# endif
#endif

void outs(char *str, size_t size) {
	if (size > 0 && !fwrite(str, size, 1, stdout)) {
		perror("fwrite(stdout)");
		exit(1);
	}
}

bool chkstr(const char *str, int (*pred)(int ch)) {
	while (*str) {
		if (!pred(*str))
			return false;

		str++;
	}

	return true;
}

uint64_t badrand64(void) {
	srandom(time(NULL));
	return random() + random() + ((random() + random()) << 32);
}

uint64_t getrand64(void) {
	uint64_t r;

#ifdef BSD
	arc4random_buf(&r, sizeof(r));
#elif defined(LINUX_GETRANDOM)
	if (getrandom(&r, sizeof(r), 0) < 0)
		r = badrand64();
#elif defined(__unix__) || defined(__unix)
	int fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0)
		r = badrand64();
	else {
		read(fd, &r, sizeof(r));
		close(fd);
	}
#else
	r = badrand64();
#endif

	return r;
}

int main(void) {
	size_t line_size = 1024;
	char *line = malloc(line_size);

	size_t line_n = 1;

	bool escaping = false;

	uint64_t id = getrand64();

	printf(
		"#ifndef FILE_%llX\n"
		"#define FILE_%llX\n"
		"\n",
		id, id
	);

	while (true) {
		ssize_t line_len = getline(&line, &line_size, stdin);

		if (line_len < 0) {
			if (feof(stdin))
				break;

			perror("getline(stdin)");
			return 1;
		}

		if (!strncmp(line, "@end", sizeof("@end")-1) && chkstr(line + sizeof("@end")-1, isspace)) {
			escaping = false;
			putchar('\n');
		} else if (escaping) {
			char esc[] = "\\\n";
			outs(esc, (size_t)(sizeof(esc)-1));
			outs(line, (size_t)(line_len-1));
		} else if (!strncmp(line, "@generic", sizeof("@generic")-1) && isspace(line[sizeof("@generic")-1])) {
			char *p = line + sizeof("@generic ")-1;

			while (isspace(*p))
				p++;

			char *tn = p;

			while (!isspace(*p)) {
				if (*p == '\0') {
					fprintf(stderr, "%zu: error: expected a type name\n", line_n);
					goto next_line;
				}
				
				p++;
			}

			char def[] = "#define ";
			outs(def, (size_t)(sizeof(def)-1));

			outs(tn, (size_t)(p - tn));

			char ty[] = "Type(";
			outs(ty, (size_t)(sizeof(ty)-1));

			do {
				while (isspace(*p))
					p++;

				if (*p == '\0')
					break;

				char *param = p;
			
				while (!isspace(*p) && *p != '\0')
					p++;

				outs(param, (size_t)(p - param));
				
				putchar(',');
			} while (*p != '\0');

			char fin[] = "T)";
			outs(fin, sizeof(fin)-1);

			escaping = true;
		} else {
			outs(line, (size_t)line_len);
		}

	next_line:
		line_n++;
	}

	printf("\n#endif\n");
}
