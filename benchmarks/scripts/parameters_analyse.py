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

#min_av = df['deviation_av'].min()
#bests_av = df.loc[df['deviation_av'] == min_av,:]


#max_score = bests_av['rebalancing_score'].max()
#bests = bests_av.loc[bests_av['rebalancing_score'] == max_score,:]


max_score = df['rebalancing_score'].max()
bests_score = df.loc[df['rebalancing_score'] == max_score,:]


min_av = bests_score['deviation_av'].min()
bests = bests_score.loc[bests_score['deviation_av'] == min_av,:]



print (df.describe())
print(bests.describe())
