#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <sys/time.h>


static
int dblcmp(const void *a, const void *b) {
    double da = *(double *)a, db = *(double *)b;
    if (da < db) {
        return -1;
    } else if (da > db) {
        return 1;
    } else {
        return 0;
    }
}

extern inline
void quick_sort(double *v, int n) {
    qsort(v, n, sizeof(double), dblcmp);
}


static inline
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

static inline
void build_min_heap(double *array, size_t n) {
	for (size_t i = (n+1)/2; i > 0; i--) {
		min_heapify(array, n, i-1);
	}
}

static inline
double extract_min(double *array, size_t n) {
	double min = array[0];

	array[0] = array[n - 1];
	min_heapify(array, n - 1, 0);

	return min;
}

static inline
double *heap_select(const double *v, int n, int k) {
    double *copy = malloc(n * sizeof(double));
    memcpy(copy, v, n * sizeof(double));

    build_min_heap(copy, n);
    double *min = malloc(k * sizeof(double));
    for (int i = 0; i < k; i++) {
        min[i] = extract_min(copy, n - i);
    }
    free(copy);
    return min;
}


#define DIFF 1E-5

static inline
bool veccmp(const double *v, const double *r, int k) {
    size_t tam = k * sizeof(double);
    double *a = memcpy(malloc(tam), r, tam);
    quick_sort(a, k);

    for (int i = 0; i < k; i++) {
        if (fabs(v[i] - a[i]) >= DIFF) {
            fprintf(stderr, "i = %d, k = %d, vi = %lf, ri = %lf, ai = %lf\n", i, k, v[i], r[i], a[i]);
            free(a);
            return false;
        }
    }
    free(a);
    return true;
}

static inline
void printvec(const double *v, int k) {
    if (k <= 0) {
        fprintf(stderr, "\n");
        return;
    }
    fprintf(stderr, "%.2lf", v[0]);
    for (int i = 1; i < k; i++) {
        fprintf(stderr, " %.2lf", v[i]);
    }
    fprintf(stderr, "\n");
}

int resposta_correta(const double *v, int n, int k, const double *r) {
    double *min = heap_select(v, n, k);
    if (veccmp(min, r, k)) {
        free(min);
        return 1;
    }
    fprintf(stderr, "Resposta incorreta detectada\n");
    fprintf(stderr, "Resposta do seu algoritmo:\n");
    printvec(r, k);
    fprintf(stderr, "Resposta esperada:\n");
    printvec(min, k);

    free(min);
    return 0;
}


static inline
double le_tempo() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    double sec = tv.tv_sec;
    double usec = tv.tv_usec;
    return sec + usec / 1E6;
}

double tempo() {
    static double ultimo = -1E9;
    double t = le_tempo();
    double diff = t - ultimo;
    ultimo = t;
    return diff;
}


void kmin_to_file(double *r, int k) {
    FILE *kmin = fopen("kmin.out", "wb");
    if (kmin == NULL) {
        perror("kmin");
        return;
    }
    size_t len = (k >= 0)? (size_t) k : 0;
    fwrite(r, sizeof(double), len, kmin);
    fclose(kmin);
}
