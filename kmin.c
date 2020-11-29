#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

/*
  Funcao para validar seus algoritmos.
  Retorna 1 se a resposta estiver correta, 0 caso contrario.
  Caso a resposta seja incorreta, imprime tambem a resposta fornecida
  e a resposta esperada.
  Parametros:
  const double *v: vetor com a instancia do problema.
  int n: numero de elementos do vetor v.
  int k: parametro k do problema (ver enunciado).
  const double *r: vetor encontrado pelo seu algoritmo, contendo nas k
  primeiras posicoes, os k menores elementos do vetor v em ordem
  crescente.
*/
int resposta_correta(const double *v, int n, int k, const double *r);

/*
  Implementacao do algoritmo QuickSort.
  Ordena o vetor passado como parametro em ordem crescente.
  Parametros:
  double *v: vetor com a instancia do problema.
  int n: numero de elementos do vetor v.
*/
void quick_sort(double *v, int n);

/*
  Utilize esta funcao para medir o tempo de execucao do seu programa.
  A primeira chamada retorna um valor indefinido. A partir da segunda
  chamada, o double retornado contem o tempo decorrido, em segundos,
  entre a chamada atual e a ultima chamada da funcao.

  Exemplo de uso para medir o tempo de execucao de uma funcao 'f()':

  tempo();
  f();
  double t = tempo();
*/
double tempo();

/*
  Escreve o vetor com os k menores elementos em um arquivo "kmin.out" criado.
  (Obrigatório o uso dessa função no final do seu código para a correção).
  Parâmetros:
  vector<double> &r: vetor encontrado pelo seu algoritmo.
*/
void kmin_to_file(double *r, int k);


/* Atributos para GCC. */
#ifdef __GNUC__
#define attribute(...)	\
	__attribute__((__VA_ARGS__))
#else
/* não faz nada em outros compiladores */
#define attribute(...)
#endif

/* * * * * * * * * * */
/* LEITURA DOS DADOS */

/* Função executada sem problemas. */
#define OK 	 0
/* Resultado da função com erro genérico. */
/* Erro específico guardado em `errno`. */
#define ERR -1
/* Erro especial para entradas inválidas. */
#define ENTINV 0x1234
#define RESERR 0x1235
#define SOLUNF 0x1236

static attribute(format(scanf, 3, 4), nonnull)
/**
 * Checked `fscanf`.
 *
 * Checa se o `fscanf` fez todas as leituras esperadas,
 * como recebido pelo parâmetro `expect`.
 *
 * Retorna true em caso de sucesso. Para erros, o valor
 * do erro é marcado em `errno` e retorna false.
 */
bool cfscanf(unsigned expect, FILE *restrict arquivo, const char *restrict fmt, ...) {
	va_list args;
	va_start(args, fmt);
	/* usa `vscanf` para tratar argumentos
	variados com mais facilidade */
	int rv = vfscanf(arquivo, fmt, args);
	va_end(args);

	/* erro de leitura */
	if (rv < 0) {
		return false;
	}
	/* leitura incompleta */
	else if (rv < expect) {
		errno = ENTINV;
		return false;
	}
	return true;
}

static attribute(nonnull, malloc, warn_unused_result)
double *read_array(const char arquivo[], size_t *n) {
	FILE *arq = fopen(arquivo, "r");
	if (arq == NULL) return NULL;

	size_t tam = *n = 0;
	if (!cfscanf(1, arq, "%zu", &tam)) {
		fclose(arq);
		return NULL;
	}

	double *arr = malloc(tam * sizeof(double));
	if (arr == NULL) {
		fclose(arq);
		return NULL;
	}

	for (size_t i = 0; i < tam; i++) {
		if (!cfscanf(1, arq, "%lf", arr + i)) {
			fclose(arq);
			free(arr);
			return NULL;
		}
	}
	*n = tam;

	fclose(arq);
	return arr;
}


static attribute(nonnull, returns_nonnull)
double *metodo_1(double *vetor, size_t n, size_t k) {
	for (size_t i = 0; i < k; i++) {
		size_t min_idx = i;
		double min_val = vetor[i];

		for (size_t j = i+1; j < n; j++) {
			double val = vetor[j];

			if (val < min_val) {
				min_idx = j;
				min_val = val;
			}
		}
		vetor[min_idx] = vetor[i];
		vetor[i] = min_val;
	}

	return vetor;
}

static attribute(nonnull, returns_nonnull)
double *metodo_2(double *vetor, size_t n, attribute(unused) size_t _k) {
	quick_sort(vetor, (int) n);
	return vetor;
}

static inline attribute(nonnull)
void min_heapify(double *vetor, size_t tam, size_t no) {
	do {
		size_t esq = 2 * no + 1;
		size_t dir = 2 * no + 2;

		size_t menor = no;
		if (esq < tam && vetor[esq] < vetor[no]) {
			menor = esq;
		}
		if (dir < tam && vetor[dir] < vetor[menor]) {
			menor = dir;
		}

		if (menor != no) {
			double tmp = vetor[menor];
			vetor[menor] = vetor[no];
			vetor[no] = tmp;

			no = menor;
		} else {
			no = tam;
		}
	} while (no < tam);
}

static attribute(nonnull)
void build_min_heap(double *array, size_t n) {
	for (size_t i = (n+1)/2; i > 0; i--) {
		min_heapify(array, n, i-1);
	}
}

static attribute(nonnull)
double extract_min(double *array, size_t n) {
	double min = array[0];

	array[0] = array[n - 1];
	min_heapify(array, n - 1, 0);

	return min;
}

static attribute(nonnull)
double *metodo_3(double *vetor, size_t n, size_t k) {
	double *min = malloc(k * sizeof(double));
	if (min == NULL) return NULL;

	build_min_heap(vetor, n);
	for (size_t i = 0; i < k; i++) {
		min[i] = extract_min(vetor, n - i);
	}
	double *fim = vetor + (n - k);
	memcpy(fim, min, k * sizeof(double));
	free(min);

	return fim;
}


typedef enum metodo {
	LIMITES = 0,
	BUSCA = 1,
	QUICKSORT = 2,
	HEAP = 3
} metodo_t;

typedef struct resultado {
	char metodo[3];
	size_t k1, k2;
} resultado_t;

static attribute(const)
resultado_t resultado_erro(void) {
	return (resultado_t) {
		.metodo = {0, 0, 0},
		.k1 = SIZE_MAX, .k2 = SIZE_MAX
	};
}

#undef NAN
#define NAN nan("")

typedef attribute(nonnull) double *(*metodo_fn)(double *, size_t, size_t);

static inline attribute(pure, nonnull)
double exec_metodo(const double *vetor, size_t n, size_t k, metodo_t metodo) {
	const metodo_fn fn[] = {[BUSCA] = metodo_1, [QUICKSORT] = metodo_2, [HEAP] = metodo_3};

	if (k >= n) {
		fprintf(stderr, "ERR: k >= n,  k = %zu, n = %zu\n", k, n);
	}

	double *copia = malloc(n * sizeof(double));
	if (copia == NULL) return NAN;
	memcpy(copia, vetor, n * sizeof(double));

	tempo();
	double *resultado = fn[metodo](copia, n, k);
	double total = tempo();

	if (resultado == NULL) {
		free(copia);
		return NAN;
	}

#ifdef CHECA_METODO_ZERO
	if (!resposta_correta(vetor, n, k, resultado)) {
		fprintf(stderr, "PROBLEMA NO METODO %d: n = %zu, k = %zu\n", metodo, n, k);
		errno = RESERR;

		free(copia);
		return NAN;
	}
#endif
	free(copia);
	return total;
}

static inline attribute(const)
size_t proximo_falsa_pos(size_t a, double ya, size_t b, double yb) {
	double xa = (double) a, xb = (double) b;

	double xn = (yb * xa - ya * xb) / (yb - ya);
	return (size_t) xn;
}


static inline attribute(pure)
ssize_t falsa_posicao(const double *vetor, size_t n, metodo_t m1, metodo_t m2) {
	size_t ka = 1, kb = n - 1;
	double fa = exec_metodo(vetor, n, ka, m1) - exec_metodo(vetor, n, ka, m2);
	double fb = exec_metodo(vetor, n, kb, m1) - exec_metodo(vetor, n, kb, m2);
	if (isnan(fa) || isnan(fb)) return SSIZE_MAX;
	if (fa * fb >= 0) {
		fprintf(stderr, "PROBLEMA NOS MÉTODOS %d, %d: ", m1, m2);
		fprintf(stderr, "impossível achar limites, ");
		fprintf(stderr, "ka = %zu, fa = %lf, kb = %zu, fb = %lf\n", ka, fa, kb, fb);
		return 0;
	}

	ssize_t maior1 = (fb > 0.0)? -1 : 1;
	while (true) {
		size_t kp = proximo_falsa_pos(ka, fa, kb, fb);
		if (kp == ka || kp == kb) {
			return maior1 * (ssize_t) kp;
		}
		double fp = exec_metodo(vetor, n, kp, m1) - exec_metodo(vetor, n, kp, m2);
		if (isnan(fp)) {
			return SSIZE_MAX;
		} else if (fp == 0.0) {
			return maior1 * (ssize_t) kp;
		} else if (fp * fa > 0.0) {
			ka = kp;
			fa = fp;
		} else {
			kb = kp;
			fb = fp;
		}
	}
}

static inline attribute(const)
metodo_t prox_metodo(metodo_t m) {
	return (m % 3) + 1;
}

static attribute(const, nonnull)
resultado_t metodo_0(const double *vetor, size_t n) {
	ssize_t k[4][4];

	for (metodo_t m1 = BUSCA; m1 <= HEAP; m1++) {
		k[m1][m1] = 0;
		for (metodo_t m2 = m1+1; m2 <= HEAP; m2++) {
			ssize_t ans = k[m1][m2] = falsa_posicao(vetor, n, m1, m2);
			k[m2][m1] = -ans;

			if (ans == SSIZE_MAX) return resultado_erro();
		}
	}

	for (metodo_t m1 = BUSCA; m1 <= HEAP; m1++) {
		metodo_t m2 = prox_metodo(m1);
		metodo_t m3 = prox_metodo(m2);

		if (k[m1][m2] <= 0 && k[m1][m3] <= 0) {
			resultado_t res;
			res.metodo[0] = m1;

			if (k[m2][m3] <= 0) {
				res.metodo[1] = m2;
				res.metodo[2] = m3;
				res.k1 = (size_t) k[m2][m1];
				res.k2 = (size_t) k[m3][m2];
			} else {
				res.metodo[1] = m3;
				res.metodo[2] = m2;
				res.k1 = (size_t) k[m2][m1];
				res.k2 = (size_t) k[m2][m3];
			}
			return res;
		}
	}
	errno = SOLUNF;
	return resultado_erro();
}

static inline attribute(nonnull)
/**
 * Apresenta o erro marcado em `errno` na saída de erro.
 */
void imprime_erro(const char prog[]) {
	switch (errno) {
		case ENTINV:
			fprintf(stderr, "%s: entrada inválida\n", prog);
			break;
		case SOLUNF:
			fprintf(stderr, "%s: não foi possível encontrar solução\n", prog);
		case RESERR:
			break;
		default:
			perror(prog);
	}
}

typedef struct args {
	const char *prog;
	metodo_t metodo;
	double *vetor;
	size_t n, k;
} args_t;

static inline attribute(nonnull)
bool parse_opt(int argc, const char *argv[], args_t *restrict args) {
	if (argc != 4) {
		if (argc == 3 && argv[2][0] != '0') {
			fprintf(stderr, "Uso Método 0: ./kmin <arq> 0\n");
			return false;
		} else if (argc != 3) {
			fprintf(stderr, "Uso geral: ./kmin <arq> <metodo> <k>\n");
			return false;
		}
	}
	const char *prog = argv[0];

	metodo_t metodo = argv[2][0] - '0';
	if (metodo < 0 || metodo > 3) {
		fprintf(stderr, "%s: método inválido\n", prog);
		return false;
	}

	size_t k = 0;
	if (argc == 4) {
		int rv = sscanf(argv[3], "%zu", &k);
		if (rv < 0) {
			perror(prog);
			return false;
		} else if (rv < 1) {
			fprintf(stderr, "%s: valor de k inválido\n", prog);
			return false;
		}
	}

	size_t tam = 0;
	double *vetor = read_array(argv[1], &tam);
	if (vetor == NULL) {
		imprime_erro(prog);
		return false;
	} else if (tam < k) {
		k = tam;
		fprintf(stderr, "%s: k maior que vetor, assumindo k = %zu\n", prog, k);
	}

	args->prog = prog;
	args->metodo = metodo;
	args->vetor = vetor;
	args->n = tam;
	args->k = k;
	return true;
}

static inline attribute(nonnull)
bool imprime_tempo(const double *restrict vetor, double *restrict resultado, size_t n, size_t k, double tempo) {
	printf("%.6f\n", tempo);
	kmin_to_file(resultado, (int) k);
	return resposta_correta(vetor, (int) n, (int) k, resultado) != 0;
}

static inline
bool imprime_klimite(resultado_t res) {
	if (res.k1 == SIZE_MAX || res.k2 == SIZE_MAX) {
		return false;
	}

	printf("M.%d >(%zu)< M.%d >(%zu)< M.%d\n",
		res.metodo[0], res.k1,
		res.metodo[1], res.k2,
		res.metodo[2]);
	return true;
}

int main(int argc, const char *argv[]){
	args_t args;
	if (!parse_opt(argc, argv, &args)) {
		return EXIT_FAILURE;
	}

	double *resultado = NULL;
	resultado_t limites = resultado_erro();
	double tempo_total = NAN;
	switch (args.metodo) {
		case LIMITES:
			limites = metodo_0(args.vetor, args.n);
			break;
		case BUSCA:
			tempo();
			resultado = metodo_1(args.vetor, args.n, args.k);
			tempo_total = tempo();
			break;
		case QUICKSORT:
			tempo();
			resultado = metodo_2(args.vetor, args.n, args.k);
			tempo_total = tempo();
			break;
		case HEAP:
			tempo();
			resultado = metodo_3(args.vetor, args.n, args.k);
			tempo_total = tempo();
			break;
	}

	int ret = EXIT_SUCCESS;
	if (!isnan(tempo_total)) {
		if (resultado == NULL) {
			imprime_erro(args.prog);
			free(args.vetor);
			return EXIT_FAILURE;
		}

		if (!imprime_tempo(args.vetor, resultado, args.n, args.k, tempo_total)) {
			free(args.vetor);
			return EXIT_FAILURE;
		}
		free(args.vetor);
	} else {
		free(args.vetor);
		if (!imprime_klimite(limites)) {
			imprime_erro(args.prog);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
