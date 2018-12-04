#This script takes client perf for a run in "client_perf" folder
#then compute the mean time and put it in a means file in the same folder 

import sys
#Arguments are: the beginning index of client VMs (this one is also the number of servers)
#and the number of client

if (len(sys.argv) != 3):
    print ("Error, arguments is asked in perf_compute: index, nb_client")
    sys.exit(-1)

nb_client= int(sys.argv[2])
index= int(sys.argv[1])

values = []
sum = 0.


for i in range (index, (index + nb_client)):
    s = "/ccc/home/cont001/ocre/billae/client_perf/vm" + str(i) + "_"
    file = open(s,"r")
    values.append(file.read())
    sum += float(values[index - i])
    file.close()

mean = sum / nb_client

#the filename match with the number of servers
filename = "/ccc/home/cont001/ocre/billae/client_perf/means_" + str(index) + "_srv"
file = open(filename, "a")

#means format:"nb_client;time\n"
file.write(str(nb_client) + ";" + str(mean) + "\n") 
