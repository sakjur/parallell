General
=======

Requirements
------------
gcc with support for C99, OpenMP and POSIX.1-2004

Compilation
-----------
`make all` compiles all the code, leaving a bunch of binaries with the file-
ending .out

Palindromic
===========

Function
--------
Palindromic takes a list of words in a LF separated list and determines which
of those words[1] occurs in their reversed form (i.e. which words are
palindromic). Examples of such words are palindromes such as "Ada" or
"tattarrattat" and semordnilaps such as "desserts" and "stressed"[2].

[1] Normalized to only lower-case letters
[2] https://en.wikipedia.org/w/index.php?title=Palindrome&oldid=704009178

Usage
-----
Run using `palindromic <input> <workers>`

<input> is mandatory and contains the name of a file which is to be read for
palindromic occurrences
<workers> is optional (default 8) and determines the number of workers to be
dispatched for finding palindromic occurences

Wordsets
--------
<words>
The words wordset was provided with the task and contains a sorted list of
25,143 words

<linux.words>
The linux.words wordset was found in `/usr/share/dict/linux.words` on a machine
running Fedora 23 and contains 479 828 words)

<wikipediapages>
The wikipediapages wordset was downloaded from Wikipedia page visit statistics
for 2016-02-01 at 14:00 and was then reduced to only the words containing only
plain ASCII characters. Upper-case characters were turned into lower-case to
make `LC_ALL=C sort` sort the list correctly. Duplicates and words longer than
80 characters were removed.

The final list contains 4,381,268 words.

Performance
-----------
Every test was run 7 times after which the smallest and the largest value was
thrown away as to eliminate extremes. The remaining 5 was summed togheter and
divided by five to get the average time of execution. Up to three most
significant digits were kept.

The performance gains on heavy workloads are significant when applying
parallelism to the problem, in both the wikipediapages and linux.words files
the time of execution is halved when running the application on three threads.

Smaller wordsets where the executions in every loop are very small
(the problem solved within each step of the main worker loop is O(log n))
have more significant overhead from thread handling than gains and starts
loosing performance after a small while.

<KTH Shell>
Red Hat Enterprise Linux 5.11
2xAMD Opteron 6172 2,1GHz(12 Cores/CPU)
82GB RAM

gcc 4.1.2
gcc -std=c99 -g -Wall -D_XOPEN_SOURCE=600 -O3 -fopenmp -o palindromic.out\
palindromic.c

# wikipediapages (4 381 268 words)
  wikipedia pages 1 worker: 5.90
  wikipedia pages 2 workers: 3.94
  wikipedia pages 3 workers: 2.82
  wikipedia pages 4 workers: 2.54
  wikipedia pages 6 workers: 2.02
  wikipedia pages 8 workers: 1.75

# linux.words (479 828 words)
  linux.words 1 worker:  0.42
  linux.words 2 workers: 0.27
  linux.words 3 workers: 0.21
  linux.words 4 workers: 0.18
  linux.words 6 workers: 0.15
  linux.words 8 workers: 0.13

# words (25 143 words)
  words 1 worker:  0.012
  words 2 workers: 0.0092
  words 3 workers: 0.0074
  words 4 workers: 0.0079
  words 6 workers: 0.0078
  words 8 workers: 0.0100

Performance, calculations
-------------------------
wp1 = 5.94292 + 5.89562 + 5.89176 + 5.89091 + 5.8883
print("wikipedia pages 1 worker: %.2f" % (wp1/5))
wp2 = 3.94263 + 3.95135 + 3.91414 + 3.93333 + 3.93992
print("wikipedia pages 2 workers: %.2f" % (wp2/5))
wp3 = 2.84798 + 2.81837 + 2.81835 + 2.76158 + 2.84241
print("wikipedia pages 3 workers: %.2f" % (wp3/5))
wp4 = 2.47633 + 2.5626 + 2.51578 + 2.56757 + 2.55783
print("wikipedia pages 4 workers: %.2f" % (wp4/5))
wp6 = 1.99245 + 2.02689 +1.98131 + 2.06752 + 2.04844
print("wikipedia pages 6 workers: %.2f" % (wp6/5))
wp8 = 1.75479 + 1.72888 + 1.76229 + 1.75646 + 1.76386
print("wikipedia pages 8 workers: %.2f" % (wp8/5))

lw1 = 0.417445 + 0.416304 + 0.417196 + 0.41593 + 0.418162
print("linux.words 1 worker:  %.2f" % (lw1/5))
lw2 = 0.270685 + 0.262586 + 0.270279 + 0.269662 + 0.269697
print("linux.words 2 workers: %.2f" % (lw2/5))
lw3 = 0.210287 + 0.211597 + 0.202043 + 0.202051 + 0.211597
print("linux.words 3 workers: %.2f" % (lw3/5))
lw4 = 0.182943 + 0.183886 + 0.184375 + 0.183633 + 0.183068
print("linux.words 4 workers: %.2f" % (lw4/5))
lw6 = 0.150371 + 0.15054 + 0.150682 + 0.149717 + 0.148549
print("linux.words 6 workers: %.2f" % (lw6/5))
lw8 = 0.134611 + 0.133569 + 0.132427 + 0.132994 + 0.132157
print("linux.words 8 workers: %.2f" % (lw8/5))

w1 = 0.011987 + 0.011936 + 0.011971 + 0.011858 + 0.011994
print("words 1 worker:  %.3f" % (w1/5))
w2 = 0.009077 + 0.009097 + 0.009228 + 0.009252 + 0.00917
print("words 2 workers: %.4f" % (w2/5))
w3 = 0.007248 + 0.007553 + 0.007255 + 0.007584 + 0.007561
print("words 3 workers: %.4f" % (w3/5))
w4 = 0.00747 + 0.007778 + 0.007195 + 0.008506 + 0.008675
print("words 4 workers: %.4f" % (w4/5))
w6 = 0.007482 + 0.006904 + 0.008872 + 0.006915 + 0.008898
print("words 6 workers: %.4f" % (w6/5))
w8 = 0.009062 + 0.010697 + 0.010394 + 0.011573 + 0.008115
print("words 8 workers: %.4f" % (w8/5))

