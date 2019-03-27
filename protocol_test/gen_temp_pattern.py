import numpy as np
import matplotlib.pyplot as plt


def gen_fun(tinit, tgrowth, thigh, tlow, yhigh, ylow, t):
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
   period = 2*tgrowth + thigh + tlow
   t = t % period
   if t <= tgrowth and tgrowth != 0: return (ylow*tgrowth + (yhigh-ylow)*t)/tgrowth
   t -= tgrowth
   # high section
   if t <= thigh: return yhigh
   t -= thigh
   # decreasing section
   if t <= tgrowth and tgrowth != 0: return (yhigh*tgrowth + (ylow-yhigh)*t)/tgrowth
   t -= tgrowth
   # low section
   return ylow 

#curve characteristics
tinit = 0
tgrowth = 5
thigh = 0
tlow = 25
yhigh = 5000
ylow = 1000

#pattern -> to -> function
temporal_size = 100
x = np.arange(temporal_size)
y = [gen_fun(tinit, tgrowth, thigh, tlow, yhigh, ylow, xi) for xi in x]

plt.plot(x,y)
plt.show()


traces = open("etc/generated_temp","w")
#function -> to -> traces
time_step = 0
while (time_step < x.size):
    pr_step = 0
    while (pr_step < y[time_step]):
        traces.write(str(x[time_step])+"\n")
        pr_step += 1
    time_step += 1

traces.close()
