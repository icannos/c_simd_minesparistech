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

repo: https://github.com/icannos/c_simd_minesparistech

We wrote 2 versions of the project: one using the join to serialize the accumulation of the results over the thread and
a second one using a mutex in each thread to handle concurrent writing at the same address. We found that the mutex 
version was the most efficient (using -O1 optimization on gcc).

We made sure to avoid false sharing in the first version but with no significant improvements.

For each we test the sequential norm on a single thread and on multithread and the vectorized norm on a single thread
and on multithread. 

It is worth to point out that the classical norm function converges to 2^24 = 16 777 216 because of the precision of the
floats: when we reach 2^24, adding <= 1 to the variable is the same as adding 0 (no-op). This is because the mantisse
is on 24 bits (23 + 1 implicit).

```.
 ├── CMakeLists.txt
 ├── main.c                   # Classic multithreading 
 ├── manual_run.sh            # To compile using gcc and run some asmples
 ├── mutex.c                  # Mutex to manage access to one variable (BEST)
 ├── nonvector.c              # Simple sum (on a separate file to remove auto vectorization)
 ├── Readme.md                # This file
 ├── run.sh                   # Compile using gcc
 └── unaligned.c              # Same as main but for non aligned data
```

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

### Usages

```bash
echo "Classic non vector"
./nonvector 33554432

echo "Standard Version 2 threads"
./projet 33554432 2
echo "Standard Version 4 threads"
./projet 33554432 4


echo "Mutex Version 2 threads"
./projetmutex 33554432 2
echo "Mutex Version 4 threads"
./projetmutex 33554432 4
```


## Non aligned data

There are 2 kinds of missalignments when adresses are not divisible by 32 (ie 4 bytes) and when we don't have a number 
of float to handle divisible by 8 (assuming we use 8 long floats vectors).

For the first problem we use the  `_mm256_loadu_ps` instruction to load each vector chunk. We adresse the second problem
using a mask to set to 0 the non-existing elements with `_mm256_maskz_load_ps`.
(If we face both problems at the same time we use `_mm256_maskz_loadu_ps`). We provide an implemention `unaligned.c`,
however for some reasons we cannot compile it because it doest not recognize above intrinsics.

## Results

Output of `./run`

```
Standard Version 2 threads
2.238724e+07
Usual scalar norm, 1 thread: 9.514435e-02
Vectorized norm, 2 thread: 6.106051e-03
Speedup x15.6
Standard Version 4 threads
2.237360e+07
Usual scalar norm, 1 thread: 9.383439e-02
Vectorized norm, 4 thread: 5.205112e-03
Speedup x18.0
Mutex Version 2 threads
2.238439e+07
Usual scalar norm, 1 thread: 9.661794e-02
Vectorized norm, 2 thread: 4.920573e-03
Speedup x19.6
Mutex Version 4 threads
2.237339e+07
Usual scalar norm, 1 thread: 9.494633e-02
Vectorized norm, 4 thread: 3.156646e-03
Speedup x30.1

```
