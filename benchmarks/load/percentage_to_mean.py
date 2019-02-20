#!/bin/env python

import sys
import pandas as pd
import matplotlib.pyplot as plt


#this script compute mean error for a given alpha value
#this script is valid there is the same line number for each column

df = pd.read_csv(sys.argv[1], delimiter=';', dtype='float32')
#n is the number of lines
n = len(df)
#n_srv is the number of columns
n_srv = len(df.columns)

total = 0

for i in range(0, n):
    for e in range(0, n_srv):
        total += abs(df.iat[i,e] - (100./n_srv))
total = total / (n*n_srv)

with open('mean.txt', 'w') as fh:
    fh.write(str(total))
