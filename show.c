#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>



static inline __attribute__((nonnull(2)))
int run(const char *file, const char *prog, char sep, int prec, FILE *out) {
	FILE *input = (file == NULL)? stdin : fopen(file, "rb");
	if (input == NULL) {
		perror(prog);
		return EXIT_FAILURE;
	}

	double number;
	size_t rv = fread_unlocked(&number, sizeof(double), 1, input);
	if (rv == 0) {
		fprintf(out, "(empty)\n");
		if (file != NULL) fclose(input);
		return EXIT_SUCCESS;
	}
	if (fprintf(out, "%.*lf", prec, number) < 0) {
		perror(prog);
		if (file != NULL) fclose(input);
		return EXIT_FAILURE;
	}

	while ((rv = fread_unlocked(&number, sizeof(double), 1, input)) > 0) {
		if (fprintf(out, "%c%.*lf", sep, prec, number) < 0) {
			perror(prog);
			if (file != NULL) fclose(input);
			return EXIT_FAILURE;
		}
	}
	if (file != NULL) fclose(input);

	if (fprintf(out, "\n") < 0) {
		perror(prog);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

static inline __attribute__((nonnull))
int parse_prec(const char *restrict text, const char *restrict prog) {
	char *end = NULL;
	long long num = strtoll(text, &end, 10);
	if (num <= LLONG_MIN || num >= LLONG_MAX) {
		perror(prog);
		return -1;
	} else if (num < 0 || num > INT_MAX) {
		fprintf(stderr, "%s: invalid precision range for option -p\n", prog);
		return -1;
	} else if (end != NULL && *end != '\0') {
		fprintf(stderr, "%s: argument '%s' is invalid for option -p\n", prog, text);
		return -1;
	}

	return (int) num;
}

int main(int argc, char *const *argv) {
	const char *prog = argv[0];
	const char *out = NULL;
	char sep = '\n';
	int precision = 2;

	for (int c; (c = getopt(argc, argv, "so:p:")) != -1;) {
		switch (c) {
			case 's':
				sep = ' ';
				break;
			case 'o':
				out = optarg;
				break;
			case 'p':
				precision = parse_prec(optarg, prog);
				if (precision < 0) {
					return EXIT_FAILURE;
				}
				break;
			case '?':
				return EXIT_FAILURE;
			default:
				abort();
		}
	}
	if (out != NULL && strcmp("-", out) == 0) {
		out = NULL;
	}
	FILE *output = (out == NULL)? stdout : fopen(out, "w");

	if (optind >= argc) {
		return run("kmin.out", prog, sep, precision, output);
	}
	for (int i = optind; i < argc; i++) {
		const char *in = (strcmp("-", argv[i]) != 0)? argv[i] : NULL;

		int rv = run(in, prog, sep, precision, output);
		if (rv != EXIT_SUCCESS) return rv;
	}
	return EXIT_SUCCESS;
}
