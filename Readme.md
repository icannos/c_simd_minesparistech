## C SIMD PROJECT -- MinesParisTech

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

### Requierments

We use cmake to configure and compile our project.

### Usage


### Using Cmake
`./run.sh` will create build directory and recompile if necessary and execute a default instance with 2^20 elements on 2 threads

When compiled, you can directly use the exec file `build/projet`:

```bash
./build/projet nb_elts nb_threads
```

### By hand compilation

```bash
gcc main.c -pthread -mavx2 -lm -O3 -o projet
./projet nb_elts nb_threads
```