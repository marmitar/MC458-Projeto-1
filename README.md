# Analysis of Algorithms I (MC458) - Project 1

- [Requirements](./Enunc.pdf)
- [Instructions](./Instructions.pdf)
- [Report](./relatorio/main.pdf)

---

## `kmin` - k-Smallest Elements Selector

Command-line program in C/C++ that, given a file with $n$ real numbers and a method flag, does one of:

| Method | Flag | What it does | Core idea |
| ------ | ---- | ------------ | --------- |
| 0 | `0` | Empirically finds cut-off values $k_1$, $k_2$ that tell which of the other methods is faster. | Adaptive benchmarking |
| 1 | `1` | Returns the $k$ smallest by repeatedly scanning and deleting the current minimum. | k linear scans |
| 2 | `2` | Sorts the full array with **QuickSort** and prints the first $k$. | Full sort |
| 3 | `3` | Builds a min-heap once and pops $k$ times. | Heap extraction |

The fastest choice depends on the ratio $k/n$; see the report for an analysis and the thresholds $k_1$, $k_2$.

---

## Building

```sh
make
```

### Running Tests

```sh
python experimentos/experimentos.py
```
