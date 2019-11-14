#!/bin/env python

import sys

#this script take time files (in arguments) and spreaded traces files
#and compute the mean time for processing a request during a period


#argument is the path of the run
if (len(sys.argv) < 4):
    print("please give the path of the run, the traces path and the number of step\n")
    sys.exit()

n_srv = 4
n_client = 12

output = sys.argv[1] + 'mean_time.csv'
time_files = sys.argv[1] + "client/vm"
trace_files = sys.argv[2]
n_step = int(sys.argv[3])

#each column is a vm
#each line is a time for a step
times = [[0] * n_step] * n_client

#each column is a vm
#each line is a number of request for a step
traces = [[0] * n_step ] * n_client


#each column is a vm
#each line is a average throughput (number of request per second) for a step
means = [[0] * n_step] * n_client

#mean of throughtput for a step
total_mean = [0] * n_step

#string to write on file
s = ''


#fill the times array
for client in range(0, n_client):
    file_name = time_files + str (client + n_srv)
    f = open (file_name, "r")
    #discard the fisrt time value
    f.readline()
    times[client] = [line.strip() for line in f.readlines()]
    f.close()


for step in range(0, n_step-1):
    zero_req = 0
    for client in range(0, n_client):
        #fill the traces array
        file_name = trace_files + "-" + str(step+1) + "-" + str (client) + ".csv"
        traces[client][step] = sum(1 for line in open(file_name))

        #compute the means array
#        if (times[client][step] == ""):
#            print(traces[client][step])
#            zero_req = zero_req + 1

        if (traces[client][step] == 0):
            means[client][step] = 0
            zero_req = zero_req + 1
        else:
            means[client][step] = float(times[client][step]) / traces[client][step]
            total_mean[step] += means[client][step]
    total_mean[step] /= (n_client - zero_req)
    s += str(total_mean[step]) + '\n'

#output
with open(output, 'w') as fh:
    fh.write(s)
