#!/bin/env python

import sys
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv(sys.argv[1], delimiter=';', dtype='float32')
#n is the number of lines
n = len(df)

totals = n * [0]
for i in range(0, n):
    for e in df:
        totals[i] += df[e][i]

#s = ';'.join(list(df)) + ';total' + '\n'
s = ''
for i in range(0, n):
    for e in df:
        s += str((df[e][i]/totals[i])*100) + ';'
    s += str(totals[i]) + '\n'

with open('results.csv', 'w') as fh:
    fh.write(s)

#df = pd.read_csv('results.csv', delimiter=';', dtype='float32')
#df.drop('total', axis=1).plot()
#plt.xlabel('Iteration')
#plt.ylabel('Percentage (%)')
#plt.ylim(0, 100)
#plt.show()
