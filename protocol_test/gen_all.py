import numpy as np
import matplotlib.pyplot as plt
import sys
import os
import glob
import subprocess
import random

###########################################################
#parameters
###########################################################
#global time (in seconds) of the trace
temporal_size = 10800

factor_distinct_key  = 0.1
path = "/ccc/scratch/cont001/ocre/billae/scratch_vm/traces/generated/MC/faible/10percent/"

#useless variable in this script but to remind /!\
nb_srv = 4
###########################################################
#curve characteristics
###########################################################
#one time step is in second (because it's timestamp unit)
tinit = 3600
tgrowth_up = 3600
tgrowth_down = 3600
thigh = 3600
tlow = 3600
# number of request when flow is high
yhigh = 18000
# number of request when flow is low
ylow = 8000
###########################################################

def balanced_repartition_per_step(size):
    """function that give an array of size elements with all same repartition"""
    array = [1/size for i in range(size)]
    return array

def random_repartition_per_step(size):
    """function that give an array of size elements randomly chosen and sum of all elements = 1"""
    array = [random.randrange(0,100) for i in range(size)]
    s = sum(array)
    for i in range(size):
        array[i] = array[i]/s
    return array

def random_temporal():
    """function that give a randomly chosen number between ylow yhigh"""
    return random.randrange(ylow,yhigh)

def gen_fun(tinit, tgrowth_up, tgrowth_down, thigh, tlow, yhigh, ylow, t):
   """function that is ylow for tinit then becomes periodic 
   __ /-\_ /-\_ /-\_ /-\_ /-\_ /-\_ /-\_ /-\_
   it grow linearly from ylow to yhigh for tgrowth
   it stays at yhigh forthigh
   it decrease linearly from yhigh to ylow for tgrowth
   it stays at ylow for tlow"""
   # init section
   if t <= tinit:  return ylow
   t -= tinit
   # increasing section
   period = tgrowth_up + thigh + tgrowth_down + tlow
   t = t % period
   if t <= tgrowth_up and tgrowth_up != 0: return (ylow*tgrowth_up + (yhigh-ylow)*t)/tgrowth_up
   t -= tgrowth_up
   # high section
   if t <= thigh: return yhigh
   t -= thigh
   # decreasing section
   if t <= tgrowth_down and tgrowth_down != 0: return (yhigh*tgrowth_down + (ylow-yhigh)*t)/tgrowth_down
   t -= tgrowth_down
   # low section
   return ylow 




#remove old files
filelist = glob.glob(path + "*")
for file in filelist:
    os.remove(file)


#generate temporal size
temporal_x = np.arange(temporal_size)
temporal_y = [gen_fun(tinit, tgrowth_up, tgrowth_down, thigh, tlow, yhigh, ylow, xi) for xi in temporal_x]
#temporal_y = [random_temporal() for xi in temporal_x]


#generate repartition of load on different serveur

#percent contain temporal_size raw and each is the repartition of load for each step
percent = [[0.] * nb_srv for i in range(temporal_size)]


file_percent = path + "repartition.csv"
#fill percent array
with open(file_percent, "w+") as fd:
    for s in range(temporal_size):
        #percent[s] = random_repartition_per_step(nb_srv)
        #percent[s] = balanced_repartition_per_step(nb_srv)
        #percent[s] = [0.85, 0.05, 0.05, 0.05]
        percent[s] = [0.3, 0.3, 0.3, 0.1]
        #print (percent[s])
        percent_str = str(percent[s]) + "\n"
        print(percent_str, file=fd)

plt.title("Description of flux temporal requests flow");
plt.xlabel("temps (sec)");
plt.ylabel("nb requests");
plt.plot(temporal_x,temporal_y)
plt.show()

file_name = path + "traces.csv"
#add the header
with open(file_name, "w+") as fd:
    fd.write("timestamp,operation,key,jobid\n")

time_step = 0
while (time_step < temporal_x.size):
    for i in range(nb_srv):
        #call c program with args: the server ID, the number request to create, the distinct_key factor, the number of available servers, a timestamp and the path of the trace file
        prog = "./bin/generator " + str(i) + " " + str(temporal_y[time_step]*percent[time_step][i]) + " " + str(factor_distinct_key) + " " + str(nb_srv) + " " + str(time_step) + " " + file_name
        #print (prog + " and repartition = " + str(percent[time_step]))
        subprocess.call(prog, shell = True)
    time_step += 1 
