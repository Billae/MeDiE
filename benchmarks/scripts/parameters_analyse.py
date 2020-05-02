#!/bin/env python3

import sys
import pandas as pd
import matplotlib.pyplot as plt


#this script compute the exhaustive parameters analyse
#It takes in arguments the path of the  deviation_recap_all.csv file
if (len(sys.argv) < 2):
    print("please give the path of the deviation_recap file\n")
    sys.exit()

file_name = sys.argv[1]
df = pd.read_csv(file_name, delimiter=';')

#remove incomplete rows
df = df.dropna()

min_av = df['score_av'].min()
bests_av = df.loc[df['score_av'] == min_av,:]

min_max = df['score_max'].min()
bests_max = df.loc[df['score_max'] == min_max,:]


print(bests_av.describe())
print(bests_max.describe())
