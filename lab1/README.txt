General
=======

Requirements
------------
clang or gcc with support for C99 and POSIX.1-2004

clang is used by default, if using gcc, run `CC=gcc make -e` or change the
line which begins with `CC=` to `CC=gcc` in Makefile

Compilation
-----------
`make all` compiles all the code, leaving a bunch of binaries with the file-
ending .out


Matrix Operations
=================

Compilation
-----------
See general. To compile only the Matrix-binaries, run `make matrix`

Variants
--------
<minmax>
matrixminmax extends the original matrix.c-application to also find the minimum
and maximum values in the matrix

<join>
matrixjoin uses pthread_join to gather the result of the matrix sub-calculations

<bag>
matrixbag uses a "bag of tasks" that encourages threads to do as much as they
possibly can rather than pre-allocating a specific number of elements every
thread handles

Usage
-----
for each of matrixminmax.out, matrixjoin.out and matrixbag.out run
`<application> <matrix_size> <workers>`

matrix_size = the number of elements in every row and column of the matrix;
default & max 10000

workers = the number of threads that divides the problem; default & max 10


QuickSort
=========

Compilation
-----------
See general. To compile only QuickSort, run `make qsort.out`

Usage
-----
`./qsort.out <workers> <elements>`

Both arguments are optional and are supposed to be entered as integers
(all-though you'll need to enter the number of workers in order to be able
to customize the number of elements)

workers = The number of threads to spawn. Default 4, Max 30
elements = The number of elements to be sorted. Default 20971520, Max 104857600

Performance
-----------

The algorithm used for parallellism is efficient enough in practice to half the
execution time for 4 threads (rather than single-threaded) on both an Intel
i5-3230M (2013) (2 cores w/ hyperthreading) and AMD Opteron 6172 (2010)
(lots of cores).

The below numbers are samples of a single run and are affected by outer factors,
but are not outliers when compared to running the same appplication multiple
times.

In order to increase performance on high number of threads (where the algorithm
doesn't keep it's performance from 4 threads), the algorithm which recursively
decends could be adapted to have n unique paths rather than 2 to avoid the
threads having to pay the performance for hitting the locked mutex.

<Laptop>
Fedora Linux 23
Intel(R) Core(TM) i5-3230M CPU @ 2.60GHz (2 cores, hyper-threading)
8 GB SO-DIMM DDR3 RAM

clang 3.7.0
clang -std=c99 -g -Wall -D_XOPEN_SOURCE=600 -O3 -lpthread -o qsort.out qsort.c

  ./qsort.out 12
  Sorted 20971520 elements using 12 worker(s)
  The execution time is 9.62403 sec

  ./qsort.out 8
  Sorted 20971520 elements using 8 worker(s)
  The execution time is 8.59107 sec

  ./qsort.out 4
  Sorted 20971520 elements using 4 worker(s)
  The execution time is 4.68837 sec

  ./qsort.out 2
  Sorted 20971520 elements using 2 worker(s)
  The execution time is 5.76749 sec

  ./qsort.out 1
  Sorted 20971520 elements using 1 worker(s)
  The execution time is 9.17274 sec

gcc 5.3.1
gcc -std=c99 -g -Wall -D_XOPEN_SOURCE=600 -O3 -lpthread -o qsort.out qsort.c

  ./qsort.out 12
  Sorted 20971520 elements using 12 worker(s)
  The execution time is 10.0317 sec

  ./qsort.out 8
  Sorted 20971520 elements using 8 worker(s)
  The execution time is 8.49735 sec

  ./qsort.out 4
  Sorted 20971520 elements using 4 worker(s)
  The execution time is 4.90241 sec

  ./qsort.out 2
  Sorted 20971520 elements using 2 worker(s)
  The execution time is 7.01231 sec

  ./qsort.out 1
  Sorted 20971520 elements using 1 worker(s)
  The execution time is 8.8639 sec

<KTH Shell>
Red Hat Enterprise Linux 5.11
2xAMD Opteron 6172 2,1GHz(12 Cores/CPU)
82GB RAM

gcc 4.1.2
gcc -std=c99 -g -Wall -D_XOPEN_SOURCE=600 -O3 -lpthread -o qsort.out qsort.c

  ./qsort.out 12
  Sorted 20971520 elements using 12 worker(s)
  The execution time is 10.3811 sec

  ./qsort.out 8
  Sorted 20971520 elements using 8 worker(s)
  The execution time is 9.66193 sec

  ./qsort.out 4
  Sorted 20971520 elements using 4 worker(s)
  The execution time is 8.36973 sec

  ./qsort.out 2
  Sorted 20971520 elements using 2 worker(s)
  The execution time is 11.2078 sec

  ./qsort.out 1
  Sorted 20971520 elements using 1 worker(s)
  The execution time is 17.6588 sec

