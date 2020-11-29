# Script para execucao dos experimentos do laboratorio 4
# Uso: python experimentos.py <executavel>
# Gera um arquivo 'resultados.dat'

import sys
import subprocess
import signal
import os
import threading

# abre arquivo de resultados
try:
    f = open('resultados.dat', 'w')
except IOError:
    print('Nao foi possivel criar o arquivo \'resultados.dat\'')
    sys.exit(1)

# nome do executavel
try:
    prog = sys.argv[1]
except IndexError:
    prog = './kmin'
if prog[0:2] != './':
    prog = f'./{prog}'
if os.path.exists(prog) == False:
    print('O arquivo executavel nao foi encontrado')
    sys.exit(1)

# nome do arquivo com a instancia
inst = 'experimentos/vet-1000000.ins'
n = 1000000
if os.path.exists(inst) == False:
    print(f'O arquivo \'{inst}\' nao foi encontrado.')
    print(f'Execute este script no mesmo diretorio do arquivo \'{inst}\'')
    sys.exit(1)

# Se demorar mais que 10 segundos,
# interrompe execucao da instancia
tmax = 5

# Classe para executar o programa
class ProgramRunner(threading.Thread):
    def __init__(self, cmd, timeout):
        threading.Thread.__init__(self)
        self.cmd = cmd
        self.timeout = timeout
        self.finished = True

    def run(self):
        self.p = subprocess.Popen(self.cmd, stdout = subprocess.PIPE, text = True)
        self.output = self.p.communicate()[0].strip().split(' ')

    def run_program(self):
        self.start()
        self.join(self.timeout)
        if self.is_alive():
            os.kill(self.p.pid, signal.SIGTERM)
            self.finished = False
            self.join()

    def get_output(self):
        if self.finished:
            return self.output
        else:
            return [str(self.timeout)]

def write_row_fmt(k, t1, t2, t3, f=sys.stdout):
    print(f'{str(k):>8s}{t1:>14s}{t2:>14s}{t3:>14s}', file=f)

# Cabecalho da saida no terminal
def print_header():
    t1, t2, t3 = [f'Tempo Met. {i+1}' for i in range(3)]
    write_row_fmt('k', t1, t2, t3)

# Impressao de uma linha no terminal
def print_row(k, t1, t2, t3):
    write_row(sys.stdout, k, t1, t2, t3)

# Escrita de uma linha no arquivo de resultados
def write_row(f, k, t1, t2, t3):
    st = [None, None, None, None]
    t = [None, t1, t2, t3]

    for i in (1, 2, 3):
        if t[i] < tmax:
            st[i] = f'{t[i]:.6f}'
        else:
            st[i] = '--------'

        j, l = (i % 3) + 1, ((i + 1) % 3) + 1

        if t[i] < t[j] and t[i] < t[l]:
            m = '*'
        else:
            m = ' '

        st[i] = ''.join([m, st[i], m])

    write_row_fmt(k, st[1], st[2], st[3], f=f)


def sweep_range(r, run_slowest):
    nrows = 0
    for k in r:
        times = [0, 0, 0]
        if run_slowest:
            algrange = range(1, 4)
        else:
            times[0] = 999
            algrange = range(2, 4)

        for alg in algrange:

            p = ProgramRunner([prog, inst, str(alg), str(k)], 2*tmax)
            p.run_program()
            output = p.get_output()

            try:
                times[alg-1] = float(output[0])
            except ValueError:
                print('')
                print(f'Saida invalida na execucao do programa {prog}')
                print(f'Instancia: {inst}')
                print(f'Metodo: {alg}')
                print(f'k = {k}')
                print('')
                sys.exit(1)

        if nrows % 20 == 0:
            print_header()
        print_row(k, times[0], times[1], times[2])
        write_row(f, k, times[0], times[1], times[2])
        nrows = nrows + 1


# Executa o programa com diferentes valores de k
sweep_range(range(1, 6), True)
sweep_range(range(1000, 1005), True)
sweep_range(range(100000, 100005), False)
sweep_range(range(300000, 300005), False)
sweep_range(range(700000, 700005), False)
sweep_range(range(999996, 1000001), False)

print('')
print('Os melhores tempos para cada valor de k foram destacados com asteriscos.')
print('O Metodo 1 foi omitido propositalmente para valores altos de k.')
print('Seus tempos de execucao sao grandes e os experimentos consumiriam muito tempo.')
print('')

f.close()
