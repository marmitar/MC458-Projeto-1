#include <cstdio>
#include <cstdlib>
#include <vector>
#include <fstream>

using namespace std;

/*
  Funcao para validar seus algoritmos.
  Retorna true se a resposta estiver correta, false caso contrario.
  Caso a resposta seja incorreta, imprime tambem a resposta fornecida
  e a resposta esperada.
  Parametros:
  const vector<double> &v: vetor com a instancia do problema.
  int k: parametro k do problema (ver enunciado).
  const vector<double> &r: vetor encontrado pelo seu algoritmo,
  contendo nas k primeiras posicoes, os k menores elementos do vetor v
  em ordem crescente.
*/
bool resposta_correta(const vector<double> &v, int k, const vector<double> &r);

/*
  Implementacao do algoritmo QuickSort.
  Ordena o vetor passado como parametro em ordem crescente.
  Parametros:
  vector<double> &v: vetor a ser ordenado.
*/
void quick_sort(vector<double> &v);

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
  Escreve o vetor r com os k menores elementos em um arquivo "kmin.out" criado.
  (Obrigatório o uso dessa função no final do seu código para a correção).
  Parâmetros:
  vector<double> &r: vetor encontrado pelo seu algoritmo.
*/
void kmin_to_file(vector<double> &r);

int main(int argc, char **argv){

    if (argc != 4) {
        if (argc == 3 && argv[2][0] != '0') {
            fprintf(stderr, "Uso Método 0: ./kmin <arq> 0\n");
            return 1;
        } else if (argc != 3) {
            fprintf(stderr, "Uso geral: ./kmin <arq> <metodo> <k>\n");
            return 1;
        }
    }

    char *nomearq = argv[1];
    char metodo = argv[2][0];
    int k;
    if (argc == 4) {
        k = atoi(argv[3]);
    }

    ifstream arq(nomearq);
    int n;

    arq >> n;

    vector<double> v(n);

    for (int i = 0; i < n; i++) {
        arq >> v[i];
    }

    arq.close();

    // Descomente para imprimir o tempo de execucao
    // printf("%.6f\n", t);

    return 0;
}
