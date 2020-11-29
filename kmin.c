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

static attribute(nonnull)
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


static inline attribute(nonnull)
array_t *copy_array(const array_t *array, size_t max) {
    size_t tam = (array->tam < max)? array->tam : max;

    array_t *copia = alloc_array(tam);
    if (copia == NULL) return NULL;

    memcpy(copia->dado, array->dado, tam * sizeof(double));
    return copia;
}

static attribute(nonnull)
array_t *metodo_1(array_t *vetor, size_t k) {
    for (size_t i = 0; i < k; i++) {
        size_t min_idx = i;
        double min_val = vetor->dado[i];

        for (size_t j = i+1; j < vetor->tam; j++) {
            double val = vetor->dado[j];

            if (val < min_val) {
                min_idx = j;
                min_val = val;
            }
        }
        vetor->dado[min_idx] = vetor->dado[i];
        vetor->dado[i] = min_val;
    }

    return copy_array(vetor, k);
}

static attribute(nonnull)
array_t *metodo_2(array_t *vetor, size_t k) {
    quick_sort(vetor->dado, (int) vetor->tam);
    return copy_array(vetor, k);
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
void build_min_heap(array_t *array) {
    size_t n = array->tam;
    for (size_t i = (n+1)/2; i > 0; i--) {
        min_heapify(array->dado, n, i-1);
    }
}

static attribute(nonnull)
double extract_min(array_t *array) {
    double min = array->dado[0];

    size_t ultimo = --array->tam;
    array->dado[0] = array->dado[ultimo];
    min_heapify(array->dado, array->tam, 0);

    return min;
}

static attribute(nonnull)
array_t *metodo_3(array_t *vetor, size_t k) {
    array_t *min = alloc_array(k);
    if (min == NULL) return NULL;

    build_min_heap(vetor);
    for (size_t i = 0; i < k; i++) {
        min->dado[i] = extract_min(vetor);
        vetor->dado[vetor->tam] = min->dado[i];
    }
    vetor->tam += 1;
    return min;
}

typedef struct resultado {
    char metodo[3];
    size_t k1, k2;
} resultado_t;

static attribute(const)
resultado_t resultado_vazio(void) {
    return (resultado_t) {
        .metodo = {'0', '0', '0'},
        .k1 = 0, .k2 = 0
    };
}

static attribute(nonnull)
resultado_t metodo_0(const array_t *vetor) {
    (void) vetor;
    return resultado_vazio();
}

static inline attribute(nonnull)
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

static inline attribute(nonnull)
bool imprime_tempo(array_t *restrict vetor, array_t *restrict resultado, double tempo) {
    printf("%.6f\n", tempo);

    kmin_to_file(resultado->dado, (int) resultado->tam);

    int rv = resposta_correta(vetor->dado, (int) vetor->tam, (int) resultado->tam, resultado->dado);
    free(resultado);

    return rv == 1;
}

static inline
bool imprime_klimite(resultado_t res) {
    if (res.metodo[0] == '0') {
        return false;
    }

    printf("%c <(%zu)< %c <(%zu)< %c\n",
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

    array_t *resultado = NULL;
    resultado_t limites = resultado_vazio();
    double tempo_total = nan("");
    switch (args.metodo) {
        case LIMITES:
            limites = metodo_0(args.vetor);
            break;
        case BUSCA:
            tempo();
            resultado = metodo_1(args.vetor, args.k);
            tempo_total = tempo();
            break;
        case QUICKSORT:
            tempo();
            resultado = metodo_2(args.vetor, args.k);
            tempo_total = tempo();
            break;
        case HEAP:
            tempo();
            resultado = metodo_3(args.vetor, args.k);
            tempo_total = tempo();
            break;
    }

    if (!isnan(tempo_total)) {
        if (resultado == NULL) {
            imprime_erro(args.prog);
            return EXIT_FAILURE;
        }

        if (!imprime_tempo(args.vetor, resultado, tempo_total)) {
            return EXIT_FAILURE;
        }
    } else if (!imprime_klimite(limites)) {
        imprime_erro(args.prog);
        return EXIT_FAILURE;
    }
    free(args.vetor);

    return EXIT_SUCCESS;
}
