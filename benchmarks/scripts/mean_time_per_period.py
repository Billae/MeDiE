#!/bin/env python

import sys

#this script take time files from mesure_perf/client and spreaded traces files from scratch_vm/traces
#and compute the mean time for processing a request during a period


#argument is the numer of servers and the number of clients 
if (len(sys.argv) < 2):
    print("please give a number of server and a number of client\n")
    sys.exit()
n_srv = int (sys.argv[1])
n_client = int (sys.argv[2])
n_step = 70

time_files = "/ccc/home/cont001/ocre/billae/mesure_perf/client/vm"
trace_files = "/ccc/scratch/cont001/ocre/billae/scratch_vm/traces/changelog-"

#each column is a vm
#each line is a time for a step
times = [[] * n_step] * n_client

#each column is a vm
#each line is a number of request for a step
traces = [[0] * n_step ] * n_client


#each column is a vm
#each line is a average throughput (number of request per second) for a step
means = [[0] * n_step] * n_client

#mean of throughtput for a step
total_mean = [0] * step

#string to write on file
s = ''

for client in range(0, n_client):
    file_name = time_files + str (client + n_srv)
    f = open (file_name, "r")
    times[client] = [line.strip() for line in f.readlines()]
    f.close()


for step in range(0, n_step):
    for client in range(0, n_client):
        #fill the traces table
        file_name = trace_files + str(step) + "-" + str (client) + ".csv"
        traces[client][step] = sum(1 for line in open(file_name))

        zero_req = 0
        #compute the means table
        if (times[client][step] == ""):
            print(traces[client][step])
            zero_req ++

        #if (traces[client][step] == 0):
        #    means[client][step] = 0
        #else:
        #    means[client][step] = float(times[client][step]) / traces[client][step]
        #    total_mean[step] += means[client][step]
    #total_mean[step] /= (n_client - zero_req)
    #s += str(total_mean[step] + '\n'

#output
#with open('mean_time.csv', 'w') as fh:
#    fh.write(s)
