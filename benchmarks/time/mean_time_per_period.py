#!/bin/env python

import sys
import pandas as pd

#this script take time files from mesure_perf/client and spreaded traces files from scratch_vm
#and compute the mean time for processing a request during a period

#argument is the numer of servers and the number of clients 
n_srv = sys.argv[1]
n_client = sys.argv[2]

for i in range(0, n_client):
    file_number = i + n_srv
    time_file = "~/mesure_perf/client/vm" + file_number
    df = pd.read_csv(time_file, delimiter=';', dtype='float32')

#output
