#!/bin/env python

import sys
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv(sys.argv[1], delimiter=';', dtype='float32')
#n is the number of lines
n = len(df)
#n_srv is the number of columns
n_srv = len(df.columns)

cumuls = n_srv * [0]
totals = n * [0]
total_all = 0

for i in range(0, n):
    for e in range(0, n_srv):
        totals[i] += df.iat[i,e]

#s = ';'.join(list(df)) + ';total' + '\n'
s = ''
for i in range(0, n):
    total_all += totals[i]
    for e in range(0, n_srv):
        cumuls[e] += df.iat[i,e]
        s += str((cumuls[e]/total_all)*100) + ';'
    s += str(total_all) + '\n'
with open('results.csv', 'w') as fh:
    fh.write(s)

#df = pd.read_csv('results.csv', delimiter=';', dtype='float32')
#df.drop('total', axis=1).plot()
#plt.xlabel('Iteration')
#plt.ylabel('Percentage (%)')
#plt.ylim(0, 100)
#plt.show()
