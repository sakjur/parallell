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

Performance
-----------

