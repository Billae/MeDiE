#!/bin/env python

import sys
import pandas as pd
import matplotlib.pyplot as plt


#this script compute mean error for a given alpha value
#this script is valid there is the same line number for each column
#It takes in arguments the path of the  run

percent_file = sys.argv[1]+"/percentages.csv"
df = pd.read_csv(percent_file, header=None, delimiter=';')
#n is the number of lines
n = len(df)
#n_srv is the number of columns
n_srv = len(df.columns)

#err_limit is the threshold indicating is the rerebalancingution is useful
err_limit = 5
useful = []
useless = []


rebalancing = [-1] * n

rebalancing_file = sys.argv[1]+"/server/rebalancing"
try:
    with open(rebalancing_file, 'r') as fd:
        lines = fd.read().splitlines()
        for line in lines:
            rebalancing[int(line)] = 1

except:
    pass

total = 0
err_max = 0
checker = 0

for i in range(0, n):
    for e in range(0, n_srv):
        err = abs(df.iat[i,e] - (100./n_srv))
        total += err
        if (err > err_max):
            err_max = err
        if (rebalancing[i] == 1):
            if (err > err_limit):
                #print("a useful rebalancing index" + str(i))
                useful.append(i)
                checker = 1

    if (rebalancing[i] == 1):
        if (checker == 0):
            useless.append(i)
        checker = 0

total = total / (n*n_srv)

#utility of rebalancing files
useful_str = '\n'.join(map(str,useful))
with open(sys.argv[1]+'/useful_rebalancing.txt', 'w') as fh:
    fh.write(useful_str)

useless_str = '\n'.join(map(str,useless))
with open(sys.argv[1]+'/useless_rebalancing.txt', 'w') as fh:
    fh.write(useless_str)

#deviation files
with open(sys.argv[1]+'/average_error.txt', 'w') as fh:
    fh.write(str(total))

with open(sys.argv[1]+'/max_error.txt', 'w') as fh:
    fh.write(str(err_max))
