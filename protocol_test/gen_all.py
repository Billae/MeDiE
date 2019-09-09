import numpy as np
import matplotlib.pyplot as plt
import sys
import subprocess
###########################################################
#parameters
###########################################################
#global time (in seconds) of the trace
temporal_size = 3

factor_distinct_key  = 0.1
path = "/ccc/scratch/cont001/ocre/billae/scratch_vm/traces/generated/test"

#percentage of requests accross servers. Sum of all = 1
nb_srv = 4
percent_0 = 0.35
percent_1 = 0.30
percent_2 = 0.30
percent_4 = 0.05

###########################################################
#curve characteristics
###########################################################
#one time step is in second (because it's timestamp unit)
tinit = 0
tgrowth_up = 0
tgrowth_down = 0
thigh = 4000
tlow = 0
# number of request when flow is high
yhigh = 1000
# number of request when flow is low
ylow = 1000
###########################################################



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


#generate temporal size
temporal_x = np.arange(temporal_size)
temporal_y = [gen_fun(tinit, tgrowth_up, tgrowth_down, thigh, tlow, yhigh, ylow, xi) for xi in temporal_x]

#plt.title("Description of flux temporal requests flow");
#plt.xlabel("temps (sec)");
#plt.ylabel("nb requests");
#plt.plot(temporal_x,temporal_y)
#plt.show()


time_step = 0
while (time_step < temporal_x.size):
    for i in range(nb_srv):
        file_name = path + str(i) + ".csv"
        #call c program with args: the server ID, the number request to create, the distinct_key factor, the number of available servers, a timestamp and the path of the trace file
        prog = "./bin/generator " + str(i) + " " + str(temporal_y[time_step]*percent_0) + " " + str(factor_distinct_key) + " " + str(nb_srv) + " " + str(time_step) + " " + file_name
        subprocess.call(prog, shell = True)
    time_step += 1 

#merge all servers files
merge = "sort -m -o " + path + "global.csv "
for i in range(nb_srv):
    merge += path + str(i) + " "

subprocess.call(merge, shell = True)
