# fit a beta distribution on the data in order to get an estimate of the minimum and maximum (support of the distribution)
# https://en.wikipedia.org/wiki/Beta_distribution
# https://stats.stackexchange.com/questions/68983/beta-distribution-fitting-in-scipy
# https://docs.scipy.org/doc/scipy/reference/generated/scipy.stats.beta.html
from scipy.stats import beta
import seaborn as sns
import matplotlib.pyplot as plot
import numpy

#prepare data
data = numpy.loadtxt(open("real_traces/sh/sklb/12_clients/servers_load.csv", "rb"), delimiter=";", skiprows=1) # use skiprows if you have an header
means = numpy.mean(data, axis=1) #mean of all servers
requests = means/300 #5*60 #in requests per second


# generate data for test
#a, b, loc, scale = 2.31, 0.627, 5, 10
#data = beta.rvs(a, b, loc, scale, size=1000)

# plot the distribution of the data
sns.set(color_codes=True)
sns.distplot(requests, hist=False, rug=True)

# fit beta distribution on data
#(a, b, loc, scale) = beta.fit(requests, loc=0)
# get minimum and maximum
#minimum = loc
#maximum = loc + scale

m = numpy.mean(requests)
s = numpy.std(requests)
minimum = m - 2*s
maximum = m + 2*s

print (minimum, maximum)

plot.show()
