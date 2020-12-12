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
difference between the manual vectorized optimization and the naive code. That is why we set the `-O1` option.

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
cd build
gcc ../main.c -O1 -fno-tree-vectorize -lpthread -lm -mavx -o projet
gcc ../mutex.c -O1 -fno-tree-vectorize -lpthread -lm -mavx -o projetmutex
gcc ../nonvector.c -O1 -fno-tree-vectorize -lpthread -lm -o nonvector
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
Standard Version 2 threads
2.238338e+07
Usual scalar norm, 1 thread: 9.516592e-02
Vectorized norm, 2 thread: 6.236485e-03
Speedup x15.3
Standard Version 4 threads
2.237157e+07
Usual scalar norm, 1 thread: 9.487285e-02
Vectorized norm, 4 thread: 5.268700e-03
Speedup x18.0
Mutex Version 2 threads
2.238682e+07
Usual scalar norm, 1 thread: 9.660557e-02
Vectorized norm, 2 thread: 4.894436e-03
Speedup x19.7
Mutex Version 4 threads
2.237339e+07
Usual scalar norm, 1 thread: 9.494633e-02
Vectorized norm, 4 thread: 4.156646e-03
Speedup x22.8

```
