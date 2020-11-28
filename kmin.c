#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
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

typedef struct array {
    size_t tam;
    double dado[];
} array_t;

static inline
array_t *alloc_array(size_t tam) {
    size_t fixo = offsetof(array_t, dado);
    size_t dados = tam * sizeof(double);

    array_t *arr = malloc(fixo + dados);
    if (arr == NULL) return NULL;

    arr->tam = tam;
    return arr;
}

static attribute(nonull)
array_t *read_array(const char arquivo[]) {
    FILE *arq = fopen(arquivo, "r");
    if (arq == NULL) return NULL;

    size_t tam = 0;
    if (!cfscanf(1, arq, "%zu", &tam)) {
        fclose(arq);
        return NULL;
    }

    array_t *arr = alloc_array(tam);
    if (arr == NULL) {
        fclose(arq);
        return NULL;
    }

    for (size_t i = 0; i < tam; i++) {
        if (!cfscanf(1, arq, "%lf", &arr->dado[i])) {
            fclose(arq);
            free(arr);
            return NULL;
        }
    }
    fclose(arq);
    return arr;
}


static inline attribute(nonull)
array_t *copy_array(const array_t *array, size_t max) {
    size_t tam = (array->tam < max)? array->tam : max;

    array_t *copia = alloc_array(tam);
    if (copia == NULL) return NULL;

    memcpy(copia->dado, array->dado, tam);
    return copia;
}

static attribute(nonull)
array_t *metodo_1(array_t *vetor, size_t k) {
    for (size_t i = 0; i < k; i++) {
        size_t min_idx = k;
        double min_val = vetor->dado[k];

        for (size_t j = k+1; j < vetor->tam; j++) {
            double val = vetor->dado[j];

            if (val < min_val) {
                min_idx = j;
                min_val = val;
            }
        }
        vetor->dado[min_idx] = vetor->dado[k];
        vetor->dado[k] = min_val;
    }

    return copy_array(vetor, k);
}

static attribute(nonull)
array_t *metodo_2(array_t *vetor, size_t k) {
    quick_sort(vetor->dado, (int) k);
    return copy_array(vetor, k);
}

static attribute(nonull)
array_t *metodo_3(array_t *vetor, size_t k) {
    return NULL;
}

static attribute(nonnull)
/**
 * Apresenta o erro marcado em `errno` na saída de erro.
 */
void imprime_erro(const char prog[]) {
	/* erro especial nesse programa */
	if (errno == ENTINV) {
		fprintf(stderr, "%s: entrada inválida\n", prog);
	/* erros gerais da libc */
	} else {
		perror(prog);
	}
}

static
double get_time(void) {
    struct timespec tp;
    if (clock_gettime(CLOCK_REALTIME, &tp) == 0) {
        return nan("");
    }

    double sec = (double) tp.tv_sec;
    double nsec = (double) tp.tv_nsec;
    return sec + (nsec / 1E9L);
}


typedef enum metodo {
    LIMITES = 0,
    BUSCA = 1,
    QUICKSORT = 2,
    HEAP = 3
} metodo_t;

typedef struct args {
    const char *prog;
    metodo_t metodo;
    array_t *vetor;
    size_t k;
} args_t;

static attribute(nonull)
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

    array_t *vetor = read_array(argv[1]);
    if (vetor == NULL) {
        imprime_erro(prog);
        return false;
    } else if (vetor->tam < k) {
        k = vetor->tam;
        fprintf(stderr, "%s: k maior que vetor, assumindo k = %zu\n", prog, k);
    }

    args->prog = prog;
    args->metodo = metodo;
    args->vetor = vetor;
    args->k = k;
    return true;
}

int main(int argc, const char *argv[]){
    args_t args;
    if (!parse_opt(argc, argv, &args)) {
        return EXIT_FAILURE;
    }

    double start_time = get_time();
    if (isnan(start_time)) {
        perror(args.prog);
        free(args.vetor);
        return EXIT_FAILURE;
    }
    switch (args.metodo) {
        case 0:
        case 1:
        case 2:
        case 3:
            break;
    }
    double end_time = get_time();
    free(args.vetor);

    if (isnan(start_time)) {
        perror(args.prog);
        return EXIT_FAILURE;
    }
    printf("%.6f\n", end_time - start_time);

    return EXIT_SUCCESS;
}
