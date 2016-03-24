#!/usr/bin/env python3
import subprocess

processes = ['./par_nlg.out', './par_sq.out', './seq_sq.out', './seq_nlg.out']
sizes = ['120', '180', '240']
cores = ['1', '2', '3', '4']

for p in processes:
  runs = []
  for size in sizes:
    if 'par' in p:
      for c in cores:
        runs.append([p, size, '50000', c])
    else:
      runs.append([p, size, '50000'])
  if 'nlg' in p:
    for run in runs:
      run.append('0.7')

      
  for run in runs:
    rv = subprocess.check_output(run).decode('utf-8')
    rv = rv.strip()
    rv = rv.split('\n')
    for i, line in enumerate(rv):
      rv[i] = line.split(' ')
    if 'workers' in rv[0]:
      cpu_count = int(rv[0][8])
    else:
      cpu_count = 1
    bodies = int(rv[0][1])
    time = float(rv[1][1])
    print("%s %s %.2f seconds %s" % (p, bodies, time, cpu_count))
