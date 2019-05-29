import numpy as np
import matplotlib.pyplot as plt
import sys

if (len(sys.argv) < 2):
    print "please give a time span for the trace file"
    exit()
#how much time the trace is representing
temporal_size = int(sys.argv[1])


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
   period = 2*tgrowth_up + thigh + tlow
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

###########################################################
#curve characteristics
###########################################################
#one time step is in second (because it's timestamp unit)
tinit = 0
tgrowth_up = 25
tgrowth_down = 5
thigh = 5
tlow = 0
# number of request when flow is high
yhigh = 5000
# number of request when flow is low
ylow = 1000
###########################################################


#pattern -> to -> function
x = np.arange(temporal_size)
y = [gen_fun(tinit, tgrowth_up, tgrowth_down, thigh, tlow, yhigh, ylow, xi) for xi in x]

#plt.plot(x,y)
#plt.show()


traces = open("/ccc/scratch/cont001/ocre/billae/scratch_vm/traces/generated/generated_temp","w")
#function -> to -> traces
time_step = 0
while (time_step < x.size):
    pr_step = 0
    while (pr_step < y[time_step]):
        traces.write(str(x[time_step])+"\n")
        pr_step += 1
    time_step += 1

traces.close()
