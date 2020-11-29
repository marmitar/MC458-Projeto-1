#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <sys/random.h>


#define unlikely(x) \
    (__builtin_expect((x), 0))


static inline __attribute__((nonnull))
size_t parse(const char *restrict text, const char *restrict prog) {
	char *end = NULL;
	long long num = strtoll(text, &end, 10);
	if (num <= LLONG_MIN || num >= LLONG_MAX) {
		perror(prog);
		return SIZE_MAX;
	} else if (num < 0 || (end != NULL && *end != '\0')) {
		fprintf(stderr, "%s: invalid argument\n", prog);
		return SIZE_MAX;
	}

	return (size_t) num;
}

static inline
void urandom(void *buf, size_t len) {
	uint8_t *ptr = buf;
	ssize_t rv = getrandom(ptr, len, GRND_RANDOM);
	size_t read = (rv < 0)? 0 : (size_t) rv;
	if (read < len) {
		getrandom(ptr + read, len - read, 0);
	}
}

static
size_t reseed(void) {
	uint16_t s1[3];
	urandom(s1, 3*2);
	seed48(s1);

	unsigned s2;
	urandom(&s2, sizeof(unsigned));
	srand(s2);

	uint16_t s3;
	urandom(&s3, 2);
	return 100 + (size_t) s3;
}

static inline
double drand(void) {
	static size_t count = 1;
	if unlikely(--count == 0) {
		count = reseed();
	}

	double num = 2 * drand48() - 1;
	return rand() * num;
}

int main(int argc, const char *argv[]) {
	const char *prog = argv[0];

	if (argc < 2) {
		fprintf(stderr, "%s: missing argument\n", prog);
		return EXIT_FAILURE;
	}

	size_t N = parse(argv[1], prog);
	if (N == SIZE_MAX) return EXIT_FAILURE;

	printf("%zu\n", N);
	for (size_t i = 0; i < N; i++) {
		printf("%lf\n", drand());
	}
	return EXIT_SUCCESS;
}
