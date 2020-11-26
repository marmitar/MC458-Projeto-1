# Script para execucao dos experimentos do laboratorio 4
# Uso: python experimentos.py <executavel>
# Gera um arquivo 'resultados.dat'

import sys
import subprocess
import signal
import os
import threading

# argumentos
if len(sys.argv) != 2:
    print 'Uso: python ' + sys.argv[0] + ' <executavel>'
    sys.exit(1)

# abre arquivo de resultados
try:
    f = open('resultados.dat', 'w')
except IOError:
    print 'Nao foi possivel criar o arquivo \'resultados.dat\''
    sys.exit(1)

# nome do executavel
prog = sys.argv[1]
if prog[0:2] != './':
    prog = './' + prog
if os.path.exists(prog) == False:
    print 'O arquivo executavel nao foi encontrado'
    sys.exit(1)

# nome do arquivo com a instancia
inst = 'vet-1000000.ins'
n = 1000000
if os.path.exists(inst) == False:
    print 'O arquivo \'' + inst + '\' nao foi encontrado.'
    print 'Execute este script no mesmo diretorio do arquivo \'' + inst + '\''
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
        self.p = subprocess.Popen(self.cmd,
                                  stdout = subprocess.PIPE)
        self.output = self.p.communicate()[0].strip().split(' ')

    def run_program(self):
        self.start()
        self.join(self.timeout)
        if self.isAlive():
            os.kill(self.p.pid, signal.SIGTERM)
            self.finished = False
            self.join()

    def get_output(self):
        if self.finished:
            return self.output
        else:
            return [str(self.timeout)]

# Cabecalho da saida no terminal
def print_header():
    print ''
    s = 'k'.rjust(8)
    s += 'Tempo Met. 1'.rjust(14)
    s += 'Tempo Met. 2'.rjust(14)
    s += 'Tempo Met. 3'.rjust(14)
    print s

# Impressao de uma linha no terminal
def print_row(k, t1, t2, t3):
    if t1 < tmax:
        st1 = str("%.6f"%t1)
    else:
        st1 = '--------'
    st2 = str("%.6f"%t2)
    st3 = str("%.6f"%t3)
    if t1 < t2 and t1 < t3:
        st1 = '*' + st1 + '*'
    elif t2 < t1 and t2 < t3:
        st2 = '*' + st2 + '*'
    else:
        st3 = '*' + st3 + '*'
    s = str(k).rjust(8)
    s += st1.rjust(14)
    s += st2.rjust(14)
    s += st3.rjust(14)
    print s

# Escrita de uma linha no arquivo de resultados
def write_row(f, k, t1, t2, t3):
    if t1 < tmax:
        st1 = str("%.6f"%t1)
    else:
        st1 = '--------'
    st2 = str("%.6f"%t2)
    st3 = str("%.6f"%t3)
    if t1 < t2 and t1 < t3:
        st1 = '*' + st1 + '*'
    elif t2 < t1 and t2 < t3:
        st2 = '*' + st2 + '*'
    else:
        st3 = '*' + st3 + '*'
    s = str(k).rjust(8)
    s += st1.rjust(14)
    s += st2.rjust(14)
    s += st3.rjust(14)
    s += '\n'
    f.write(s)


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
                print ''
                print 'Saida invalida na execucao do programa ' + prog
                print 'Instancia: ' + inst
                print 'Metodo: ' + str(alg)
                print 'k = ' + str(k)
                print ''
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

print ''
print 'Os melhores tempos para cada valor de k foram destacados com asteriscos.'
print 'O Metodo 1 foi omitido propositalmente para valores altos de k.'
print 'Seus tempos de execucao sao grandes e os experimentos consumiriam muito tempo.'
print ''

f.close()
