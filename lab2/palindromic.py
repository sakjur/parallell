#!/usr/bin/env python3

words = {}

reverse = lambda w: w[::-1]

with open("/home/sakjur/wikipediapages") as f:
  for line in f:
    line = line.lower()
    line = line.strip()
    words[line] = True

for word in words:
  if reverse(word) in words:
    print(word)
