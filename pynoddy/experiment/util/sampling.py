# -*- coding: utf-8 -*-
"""
Created on Fri Jul 03 14:20:22 2015
Utility functions for sampling variables from a variety of
probability distributions, given means and 95 percentile intervals (ci)
(ie. 95% of all sampled values fall within mean ± ci)
@author: Sam Thiele
"""

import numpy as np
import scipy as sc
import scipy.stats


# build a lookup table for kappa values given a 95 percentile interval


def VMLookupTable():
    try:  # try loading ordered dict
        from collections import OrderedDict
    except ImportError:  # not installed, try on pythonpath
        try:
            from OrderedDict import OrderedDict
        except ImportError:
            "PyNoddy requires OrderedDict to run. Please download it and make it available on the pythonpath."

    kappa_lookup = OrderedDict()


    #disable numpy warnings
    err = np.geterr()
    np.seterr(all='ignore')
    
    
    # build lookup table
    for k in range(1000, 100, -20):
        ci = sc.stats.vonmises.interval(0.95, k)
        kappa_lookup[ci[1]] = k
    for k in range(100, 10, -1):
        ci = sc.stats.vonmises.interval(0.95, k)
        kappa_lookup[ci[1]] = k
    for k in np.arange(10, 0, -0.1):
        ci = sc.stats.vonmises.interval(0.95, k)
        kappa_lookup[ci[1]] = k

    #re-enable numpy warnings
    np.seterr(**err)
    
    # return lookup table
    return kappa_lookup


# build (static) lookup table
kappa_lookup = VMLookupTable()


def findKappa(ci):
    if (kappa_lookup.has_key(ci)):  # this value has been calculated
        return kappa_lookup[ci]
    else:  # we need to interpolate
        return np.interp(ci, kappa_lookup.keys(), kappa_lookup.values())


# draw sample from a specified von-mises distribution
def VonMises(mean, ci, count):
    mu = np.radians(mean)
    # stdev = ci / 1.959963984540
    # kappa = 1.0 / pow(np.radians(stdev),2) #kappa approximates 1 / variance
    kappa = findKappa(np.radians(ci))

    if (count == 1):  # single sample, return single value
        return np.degrees(np.random.vonmises(mu, kappa, count))[0]
    else:  # multi sample, return array
        return np.degrees(np.random.vonmises(mu, kappa, count))


# draw sample from a specified normal distribution
def Normal(mean, ci, count):
    stdev = ci / 1.959963984540

    if count == 1:  # return single value
        return np.random.normal(mean, stdev, count)[0]
    else:  # return list of values
        return np.random.normal(mean, stdev, count)


# draw sample from a specified uniform distribution
def Uniform(mean, ci, count):
    # calculate min/max
    minimum = mean - ci * 1.025  # 5% (0.05) of range (max - min) is outside of the confidence interval
    maximum = mean + ci * 1.025

    if (count == 1):  # return single value
        return np.random.uniform(minimum, maximum, count)[0]
    else:  # return multiple values
        return np.random.uniform(minimum, maximum, count)


# prints the percentiles of given sample
def printPercentiles(x):
    mean = np.mean(x)
    print("mean = %f" % mean)
    print("2.5 percentile = %f" % np.percentile(x, 2.5))
    print("97.5 percentile = %f" % np.percentile(x, 97.5))


# test function
def test():
    x = Normal(100, 15, 100000)  # sample normal distribution
    y = VonMises(100, 15, 100000)  # sample von mises distribution
    z = Uniform(100, 15, 100000)  # sample uniform distribution

    # output
    print("Normal")
    printPercentiles(x)
    print("Von-Mises")
    printPercentiles(y)
    print("Uniform")
    printPercentiles(z)
