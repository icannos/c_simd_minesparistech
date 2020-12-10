# C SIMD PROJECT -- MinesParisTech

```
 _____    _          _              _____                 ___  ___           _                 ______
|  ___|  | |        (_)            /  __ \       ___      |  \/  |          (_)                |  _  \
| |__  __| |_      ___  __ _  ___  | /  \/      ( _ )     | .  . | __ ___  ___ _ __ ___   ___  | | | |
|  __|/ _` \ \ /\ / / |/ _` |/ _ \ | |          / _ \/\   | |\/| |/ _` \ \/ / | '_ ` _ \ / _ \ | | | |
| |__| (_| |\ V  V /| | (_| |  __/ | \__/\_    | (_>  <   | |  | | (_| |>  <| | | | | | |  __/ | |/ /
\____/\__,_| \_/\_/ |_|\__, |\___|  \____(_)    \___/\/   \_|  |_/\__,_/_/\_\_|_| |_| |_|\___| |___(_)
                        __/ |                                                                         
                       |___/                                                                          
```

## Introduction

We wrote 2 versions of the project: one using the join to serialize the accumulation of the results over the thread and
a second one using a mutex in each thread to handle concurrent writing at the same address.

For each we test the sequential norm on a single thread and on multithread and the vectorized norm on a single thread
and on multithread. Our programm outputs the value of the calculated norm (to check correctness) and then the 
computation time.

The differences in the values we get are floating-point errors due to the non associativity of the computations.

## Requierments

We use cmake to configure and compile our project.

## Notes about the compilation

It is worth to point out that gcc will automatically vectorize loops using vector instructions thus reducing the 
difference between the manual vectorized optimization and the naive code. That is why we set the `-O0` option.

## Usage


### Using Cmake
`./run.sh` will create build directory and recompile if necessary and execute a default instance with 2^20 elements on 2 threads

When compiled, you can directly use the exec file `build/projet`:

```bash
./build/projet nb_elts nb_threads
./build/projetmutex nb_elts nb_threads
```

### By hand compilation

```bash
gcc main.c -pthread -mavx2 -lm -O0 -o projet
./projet nb_elts nb_threads
```

```bash
gcc mutex.c -pthread -mavx2 -lm -O0 -o projetmutex
./projetmutex nb_elts nb_threads
```

## Non aligned data

There are 2 kinds of missalignments when adresses are not divisible by 32 (ie 4 bytes) and when we don't have a number 
of float to handle divisible by 8 (assuming we use 8 long floats vectors).

For the first problem we use the  `_mm256_loadu_ps` instruction to load each vector chunk. We adresse the second problem
using a mask to set to 0 the non-existing elements with `_mm256_maskz_load_ps`. 
(If we face both problems at the same time we use `_mm256_maskz_loadu_ps`).

## Results

Output of `./run`

```
## Standard Version
6.988261e+05
6.987406e+05
6.987094e+05
6.987089e+05
Usual scalar norm, 1 thread: 4.038417e-03
Usual scalar norm, 2 thread: 2.291911e-03
Vectorized norm, 1 thread: 1.157755e-03
Vectorized norm, 2 thread: 8.525660e-04
## Mutex Version
6.988261e+05
6.987406e+05
6.987094e+05
6.987089e+05
Usual scalar norm, 1 thread: 3.276267e-03
Usual scalar norm, 2 thread: 1.974854e-03
Vectorized norm, 1 thread: 1.317278e-03
Vectorized norm, 2 thread: 6.261590e-04
```
