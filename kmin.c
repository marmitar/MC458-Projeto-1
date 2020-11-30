/**
 * Tiago de Paula Alves - RA187679
 * Projeto de Algoritmo com Implementação 1
 *
 * Código implementado em C99.
 * * * * * * * * * * * * * * * * * * * * * */
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


/* Atributos para GCC. */
#ifdef __GNUC__
#define attribute(...)	\
	__attribute__((__VA_ARGS__))
#else
// não faz nada em outros compiladores
#define attribute(...)
#endif


/* * * * * * * *
 * ROTINAS_C.O */

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


/* * * * * * * * * * */
/* LEITURA DOS DADOS */

/* Erros especiais. */
// ENTrada INVálida
#define ENTINV 0x1234
// RESposta com ERRo
#define RESERR 0x1235
// SOLUção Não Encontrada
#define SOLUNE 0x1236

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
	// usa `vfscanf` para tratar argumentos
	// variados com mais facilidade
	int rv = vfscanf(arquivo, fmt, args);
	va_end(args);

	// erro de leitura
	if (rv < 0) {
		return false;
	}
	// leitura incompleta
	else if (rv < expect) {
		errno = ENTINV;
		return false;
	}
	return true;
}

static attribute(nonnull, malloc, warn_unused_result)
/**
 * Leitura do vetor a partir de um arquivo texto.
 *
 * Escreve o tamanho em `n`.
 */
double *read_array(const char arquivo[], size_t *n) {
	FILE *arq = fopen(arquivo, "r");
	if (arq == NULL) return NULL;

	size_t tam = *n = 0;
	// primeira linha é o tamanho
	if (!cfscanf(1, arq, "%zu", &tam)) {
		fclose(arq);
		return NULL;
	}

	// alocação do vetor resultado
	double *arr = malloc(tam * sizeof(double));
	if (arr == NULL) {
		fclose(arq);
		return NULL;
	}

	// leitura dos elementos
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


/* * * * * * * * * * * */
/* TRATAMENTO DA SAÍDA */

// Enum dos métodos.
typedef enum metodo {
	LIMITES = 0,
	BUSCA = 1,
	QUICKSORT = 2,
	HEAP = 3
} metodo_t;

// Resultado do método 0
typedef struct resultado {
	// Ordem dos métodos
	metodo_t metodo[3];
	// Limites onde cada um é melhor
	size_t k1, k2;
} resultado_t;

static attribute(const)
/**
 * Marcador de erro do método 0.
 */
resultado_t resultado_erro(void) {
	return (resultado_t) {
		.metodo = {0, 0, 0},
		.k1 = SIZE_MAX, .k2 = SIZE_MAX
	};
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
		case SOLUNE:
			fprintf(stderr, "%s: não foi possível encontrar solução\n", prog);
		case RESERR:
			break;
		default:
			perror(prog);
	}
}

static inline attribute(nonnull)
/**
 * Saída dos métodos de ordenação.
 */
bool imprime_tempo(const double *restrict vetor, double *restrict resultado, size_t n, size_t k, double tempo) {
	// tempo de execução
	printf("%.6f\n", tempo);
	// resultado do teste
	kmin_to_file(resultado, (int) k);
	// checagem para depuração
#ifdef CHECA_METODOS
	return resposta_correta(vetor, (int) n, (int) k, resultado) != 0;
#else
	return true;
#endif
}

static inline attribute(const)
/**
 * Número de dígitos decimais de um número.
 */
int digitos(size_t num) {
	int cnt = 0;
	while (num > 0) {
		num /= 10;
		cnt++;
	}
	return cnt;
}

static inline
/**
 * Saída do método 0.
 */
bool imprime_k(resultado_t res, size_t n) {
	// checa resultado com erro
	for (size_t i = 0; i < 3; i++) {
		if (res.metodo[i] == LIMITES) {
			return false;
		}
	}
	// casas para alinhamento dos limites
	int prec = digitos(n);
	// limites dos intervalos
	size_t k[] = {0, res.k1, res.k2, n};

	printf("Vetor de tamanho  %zu\n\n", n);
	// tabela com os métodos e os limites
	printf("Método    Intervalo Eficiente\n");
	for (unsigned i = 0; i < 3; i++) {
		printf("     %d    %*zu até %*zu\n",
			res.metodo[i], prec, k[i], prec, k[i+1]);
	}

	return true;
}


/* * * * */
/* MAIN  */

// Marcador de double inválido.
#undef NAN
#define NAN nan("")

// Argumentos da linha de comando
typedef struct args {
	// Nome do programa
	const char *prog;
	// Método escolhido
	metodo_t metodo;
	// Dados do teste
	double *vetor;
	size_t n, k;
} args_t;

static inline attribute(nonnull)
/**
 * Parser dos argumentos da linha de comandos.
 */
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

	// validação do método
	metodo_t metodo = argv[2][0] - '0';
	if (metodo < 0 || metodo > 3 || argv[2][1] != '\0') {
		fprintf(stderr, "%s: método inválido\n", prog);
		return false;
	}

	// parseia k quando dado
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

	// leitura do vetor
	size_t tam = 0;
	double *vetor = read_array(argv[1], &tam);
	if (vetor == NULL) {
		imprime_erro(prog);
		return false;
	// k muito grande
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

/* Declaração dos métodos. */
static
resultado_t metodo_0(const double *vetor, size_t n)
attribute(const, nonnull);
static
double *metodo_1(double *vetor, size_t n, size_t _k)
attribute(nonnull);
static
double *metodo_2(double *vetor, size_t n, size_t _k)
attribute(nonnull);
static
double *metodo_3(double *vetor, size_t n, size_t _k)
attribute(nonnull);

/* Main */
int main(int argc, const char *argv[]){
	args_t args;
	if (!parse_opt(argc, argv, &args)) {
		return EXIT_FAILURE;
	}

	// resultado dos métodos de ordenação
	double *resultado = NULL;
	// resultado do método 0
	resultado_t limites = resultado_erro();
	// tempo de execução, métodos 1, 2, 3
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

	// tempo medido: foi executado método 1, 2 ou 3
	if (!isnan(tempo_total)) {
		if (resultado == NULL) {
			// método retornou erro
			imprime_erro(args.prog);
			free(args.vetor);
			return EXIT_FAILURE;
		}

		if (!imprime_tempo(args.vetor, resultado, args.n, args.k, tempo_total)) {
			// método com resultado errado
			free(args.vetor);
			return EXIT_FAILURE;
		}
		free(args.vetor);
	// tempo não medido: método 0
	} else {
		free(args.vetor);

		if (!imprime_k(limites, args.n)) {
			// erro na medida dos limites
			imprime_erro(args.prog);
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}


/* * * * * * */
/* MÉTODO  0 */

// maior valor de `k * n` que roda o método 1
#define MAX_BUSCA  0x12A05F200ULL
// resultado do tempo se `k * n` for maior
#define TEMPO_MAX_BUSCA  3.6E3

typedef intptr_t ssize_t;

// Ponteiro para os métodos 1, 2 ou 3.
typedef attribute(nonnull) double *(*metodo_fn)(double *, size_t, size_t);

static inline attribute(pure, nonnull)
/**
 * Executa um método e retorna o tempo de execução ou NaN em caso de erro.
 */
double exec_metodo(const double *vetor, size_t n, size_t k, metodo_t metodo) {
	// método 1 demora muito para valores grandes de k e n
	if (metodo == BUSCA && k * n >= MAX_BUSCA) {
		return TEMPO_MAX_BUSCA;
	}
	// vetor com os métodos
	const metodo_fn fn[] = {[BUSCA] = metodo_1, [QUICKSORT] = metodo_2, [HEAP] = metodo_3};

	// copia o vetor para não alterar o original
	double *copia = malloc(n * sizeof(double));
	if (copia == NULL) return NAN;
	memcpy(copia, vetor, n * sizeof(double));

	// mede tempo
	tempo();
	double *resultado = fn[metodo](copia, n, k);
	double total = tempo();
	// resultado de erro
	if (resultado == NULL) {
		free(copia);
		return NAN;
	}

	// checa se resultado correto
#ifdef CHECA_METODOS
	if (!resposta_correta(vetor, n, k, resultado)) {
		fprintf(stderr, "PROBLEMA NO METODO %d: n = %zu, k = %zu\n", metodo, n, k);
		errno = RESERR;

		free(copia);
		return NAN;
	}
#endif
	// retorna o tempo de execução
	free(copia);
	return total;
}

static inline attribute(const)
/**
 * Próximo `x` no método da falsa posição
 */
size_t proximo_falsa_pos(size_t a, double ya, size_t b, double yb) {
	double xa = (double) a, xb = (double) b;
	double xn = (yb * xa - ya * xb) / (yb - ya);
	return (size_t) xn;
}

static inline attribute(pure)
/**
 * Implementação do método da falsa posição para achar
 * o k onde um dos métodos é maior que o outro.
 *
 * Retorna valor negativo se antes do k o método m1 é mais
 * rápido ou positivo em se for o m2. Retorna 0 em erro.
 */
ssize_t falsa_posicao(const double *vetor, size_t n, metodo_t m1, metodo_t m2) {
	size_t ka = 1, kb = n;
	double fa = exec_metodo(vetor, n, ka, m1) - exec_metodo(vetor, n, ka, m2);
	double fb = exec_metodo(vetor, n, kb, m1) - exec_metodo(vetor, n, kb, m2);
	// checagem de erro
	if (isnan(fa) || isnan(fb)) return 0;

	// sinal do resultado
	ssize_t maior1 = (fb > 0.0)? -1 : 1;
	// um dos métodos é sempre maior que o outro
	if (fa * fb >= 0) {
		return maior1;
	}

	while (true) {
		// proximo passo
		size_t kp = proximo_falsa_pos(ka, fa, kb, fb);
		// para se chegar nos limites
		if (kp <= ka || kp >= kb) {
			return maior1 * (ssize_t) kp;
		}

		// diferença do tempo no proximo passo
		double fp = exec_metodo(vetor, n, kp, m1) - exec_metodo(vetor, n, kp, m2);
		// valor com erro
		if (isnan(fp)) {
			return 0;
		// transição exata
		} else if (fp == 0.0) {
			return maior1 * (ssize_t) kp;
		// atauliza os limites
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
/**
 * Próximo método: 1 -> 2 -> 3 -> 1
 */
metodo_t prox_metodo(metodo_t m) {
	return (m % 3) + 1;
}

static attribute(const, nonnull)
/**
 * Implementação do método 0.
 */
resultado_t metodo_0(const double *vetor, size_t n) {
	// resultado qualquer para vetor vazio
	if (n == 0) {
		return (resultado_t) {
			.metodo = {BUSCA, QUICKSORT, HEAP},
			.k1 = 0, .k2 = 0
		};
	}

	// encontra os limites para cada para para de métodos
	ssize_t k[4][4];
	for (metodo_t m1 = BUSCA; m1 <= HEAP; m1++) {
		// mesmo método deveria ter resultados iguais
		k[m1][m1] = 0;
		// compara com os outros metodos
		for (metodo_t m2 = m1+1; m2 <= HEAP; m2++) {
			ssize_t ans = k[m1][m2] = falsa_posicao(vetor, n, m1, m2);
			// o contrário, basta inverter o sinal
			k[m2][m1] = -ans;

			if (ans == 0) return resultado_erro();
		}
	}

	// encontra o método que executa melhor com k menor
	for (metodo_t m1 = BUSCA; m1 <= HEAP; m1++) {
		// metodos seguintes
		metodo_t m2 = prox_metodo(m1);
		metodo_t m3 = prox_metodo(m2);

		// se m1 executa mais rápido que m2 e m3 no começo
		if (k[m1][m2] <= 0 && k[m1][m3] <= 0) {
			resultado_t res;
			res.metodo[0] = m1;

			// escolha entre os outros dois métodos
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
	// solução não encontrada
	errno = SOLUNE;
	return resultado_erro();
}


/* * * * * * */
/* MÉTODO  1 */

static attribute(nonnull)
/**
 * Busca linear de cada i-ésimo menor.
 */
double *metodo_1(double *vetor, size_t n, size_t k) {
	// para cada i
	for (size_t i = 0; i < k; i++) {
		size_t min_idx = i;
		double min_val = vetor[i];

		// busca o menor nos n-i elementos a seguir
		for (size_t j = i+1; j < n; j++) {
			double val = vetor[j];

			if (val < min_val) {
				min_idx = j;
				min_val = val;
			}
		}
		// troca com o i-esimo
		vetor[min_idx] = vetor[i];
		vetor[i] = min_val;
	}
	return vetor;
}


/* * * * * * */
/* MÉTODO  2 */

static attribute(nonnull)
/**
 * Quicksort para achar os k menores.
 */
double *metodo_2(double *vetor, size_t n, attribute(unused) size_t _k) {
	quick_sort(vetor, (int) n);
	return vetor;
}


/* * * * * * */
/* MÉTODO  3 */

static inline attribute(nonnull)
/**
 * Heapify do MinHeap.
 */
void min_heapify(double *vetor, size_t tam, size_t no) {
	do {
		// filhos do no
		size_t esq = 2 * no + 1;
		size_t dir = 2 * no + 2;

		// menor nó entre pai e filhos
		size_t menor = no;
		if (esq < tam && vetor[esq] < vetor[no]) {
			menor = esq;
		}
		if (dir < tam && vetor[dir] < vetor[menor]) {
			menor = dir;
		}

		// troca com pai com o menor
		if (menor != no) {
			double tmp = vetor[menor];
			vetor[menor] = vetor[no];
			vetor[no] = tmp;

			no = menor;
		// ou encerra recursão
		} else {
			no = tam;
		}
	} while (no < tam);
}

static attribute(nonnull)
/**
 * Monta MinHeap a partir de um vetor.
 */
void build_min_heap(double *array, size_t n) {
	for (size_t i = (n+1)/2; i > 0; i--) {
		min_heapify(array, n, i-1);
	}
}

static attribute(nonnull)
/**
 * Extração do menor elemento do heap.
 */
double extract_min(double *array, size_t n) {
	double min = array[0];

	array[0] = array[n - 1];
	min_heapify(array, n - 1, 0);

	return min;
}

static attribute(nonnull)
/**
 * Acha k menores usando um heap de mínimo.
 */
double *metodo_3(double *vetor, size_t n, size_t k) {
	// vetor com os k menores
	double *min = malloc(k * sizeof(double));
	if (min == NULL) return NULL;

	// ordena em heap e recupera os k menores
	build_min_heap(vetor, n);
	for (size_t i = 0; i < k; i++) {
		min[i] = extract_min(vetor, n - i);
	}
	// copia o resultado em ordem para o final do vetor
	// mantendo os elementos que existiam lá
	double *fim = vetor + (n - k);
	memcpy(fim, min, k * sizeof(double));
	free(min);

	// retorna o ponteiro pro final do vetor
	return fim;
}

