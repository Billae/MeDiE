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

min_score = df['score'].min()
bests = df.loc[df['score'] == min_score,:]

print(bests.describe())
