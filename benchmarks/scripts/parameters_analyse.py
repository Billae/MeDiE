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

#remove incomplete rows and runs without rebalancing
df = df.dropna()
df = df[df.redistribution_interval != 300]
print (df.describe())

rebalancing_score = df['redistribution_useless'] / (df['redistribution_useful'] + df['redistribution_useless']) 
#normalization = (s -smin) / (smax - smin)
if ((df['deviation_av'].max() - df['deviation_av'].min()) == 0):
    deviation_av_score = df['deviation_av']
else:
    deviation_av_score = ( df['deviation_av'] - df['deviation_av'].min() ) / (df['deviation_av'].max() - df['deviation_av'].min()) 

if ((df['deviation_max'].max() - df['deviation_max'].min()) == 0):
    deviation_max_score = df['deviation_max']
else:
    deviation_max_score = ( df['deviation_max'] - df['deviation_max'].min() ) / (df['deviation_max'].max() - df['deviation_max'].min()) 


all_score = 0.5 * rebalancing_score + 0.5 * deviation_av_score

optional = pd.concat([rebalancing_score, deviation_av_score, deviation_max_score, all_score],axis=1 )
optional.columns = ['rebalancing_score', 'deviation_av_score', 'deviation_max_score', 'all_score']
print (optional.describe())

newdf = pd.concat([df, optional], axis=1)

#min_av = df['deviation_score_av'].min()
#bests_av = df.loc[df['deviation_score_av'] == min_av,:]


min_all = newdf['all_score'].min()
bests = newdf.loc[newdf['all_score'] == min_all,:]


#max_score = bests_av['rebalancing_score'].max()
#bests = bests_av.loc[bests_av['rebalancing_score'] == max_score,:]


#max_score = df['rebalancing_score'].max()
#bests_score = df.loc[df['rebalancing_score'] == max_score,:]


#min_av = bests_score['deviation_av'].min()
#bests = bests_score.loc[bests_score['deviation_av'] == min_av,:]



#print (newdf.describe())
print(bests[['alpha','N_entry', 'redistribution_interval','rebalancing_score', 'deviation_av_score', 'all_score']].describe())
