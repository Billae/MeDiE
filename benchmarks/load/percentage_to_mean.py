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
err_max = 0

for i in range(0, n):
    for e in range(0, n_srv):
        err = abs(df.iat[i,e] - (100./n_srv))
        total += err
        if (err > err_max):
            err_max = err

total = total / (n*n_srv)

with open('average_error.txt', 'w') as fh:
    fh.write(str(total))

with open('max_error.txt', 'w') as fh:
    fh.write(str(err_max))
