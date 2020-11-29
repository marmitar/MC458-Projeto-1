#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>



static inline __attribute__((nonnull(2, 3)))
int run(const char *file, const char *prog, const char *sep, int prec) {
	FILE *input = (file == NULL)? stdin : fopen(file, "rb");
	if (input == NULL) {
		perror(prog);
		return EXIT_FAILURE;
	}

	double number;
	size_t rv = fread_unlocked(&number, sizeof(double), 1, input);
	if (rv == 0) {
		if (file != NULL) fclose(input);
		return (printf("(empty)\n") < 0)? EXIT_FAILURE : EXIT_SUCCESS;
	}
	if (printf("%.*lf", prec, number) < 0) {
		perror(prog);
		if (file != NULL) fclose(input);
		return EXIT_FAILURE;
	}

	while ((rv = fread_unlocked(&number, sizeof(double), 1, input)) > 0) {
		if (printf("%s%.*lf", sep, prec, number) < 0) {
			perror(prog);
			if (file != NULL) fclose(input);
			return EXIT_FAILURE;
		}
	}
	if (file != NULL) fclose(input);

	if (printf("\n") < 0) {
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
		fprintf(stderr, "%s: invalid precision range -- 'p'\n", prog);
		return -1;
	} else if (end != NULL && *end != '\0') {
		fprintf(stderr, "%s: invalid argument -- 'p'\n", prog);
		return -1;
	}

	return (int) num;
}

static __attribute__((nonnull))
void usage(const char prog[]) {
	printf("Leitura do vetor de saída, normalmente 'kmin.out'\n");
	printf("\n");
	printf("Uso: %s [-p PREC] [-s [SEP]] [entrada ...]\n", prog);
	printf("\n");
	printf("Opções:\n");
	printf("  -h          Mensagem de ajuda.\n");
	printf("  -p PREC     Precisão dos valores. (PADRÃO: 2)\n");
	printf("  -s [SEP]    Muda o separador de valores.\n");
	printf("              (PADRÃO: quebra de linha)\n");
	printf("              (SEM ARGUMENTO: espaço)\n");
}

int main(int argc, char *const *argv) {
	const char *prog = argv[0];
	const char *sep = "\n";
	int precision = 2;

	for (int c; (c = getopt(argc, argv, "p:s::h")) != -1;) {
		switch (c) {
			case 'h':
				usage(prog);
				return EXIT_SUCCESS;
			case 'p':
				precision = parse_prec(optarg, prog);
				if (precision < 0) {
					return EXIT_FAILURE;
				}
				break;
			case 's':
				sep = (optarg != NULL)? optarg : " ";
				break;
			case '?':
				return EXIT_FAILURE;
			default:
				abort();
		}
	}

	if (optind >= argc) {
		return run("kmin.out", prog, sep, precision);
	}
	for (int i = optind; i < argc; i++) {
		const char *in = (strcmp("-", argv[i]) != 0)? argv[i] : NULL;

		int rv = run(in, prog, sep, precision);
		if (rv != EXIT_SUCCESS) return rv;
	}
	return EXIT_SUCCESS;
}
