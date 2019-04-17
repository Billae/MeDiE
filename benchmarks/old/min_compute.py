#This script takes the maximum value of a mean_x_srv file
#then put it in a max file in the same folder

import sys
import re
#Argument is the name of the file to open

if (len(sys.argv) != 2):
    print ("Error, argument is asked in max_compute: file name")
    sys.exit(-1)

file_mean = open(sys.argv[1], "r")
expr = re.search("(.)+means_(?P<nb>\d+)_srv", sys.argv[1])
nb_srv = expr.group('nb')


t = re.search("(?P<nbc>\d*);(?P<time>.+)\n", file_mean.readline())
mini = t.group('time')
minnbc = t.group('nbc')
print mini

file_mean = open(sys.argv[1], "r")
for line in file_mean:
    t = re.search("(?P<nbc>\d*);(?P<time>.+)", line)
    if (float(t.group('time')) < mini):
        mini = float(t.group('time'))
        minnbc = t.group('nbc')
        print t.group('time')

file_mean.close()

file_min = open ("/ccc/home/cont001/ocre/billae/client_perf/min", "a")
file_min.write(str(nb_srv) + ";" + minnbc + ";" + str(mini) + "\n")
file_min.close()
