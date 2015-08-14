#! /usr/bin/env python

from __future__ import print_function
from openturns import *

TESTPREAMBLE()
RandomGenerator.SetSeed(0)

try:
    distribution = Exponential(2.5, -1.3)
    size = 10000
    sample = distribution.getSample(size)
    factory = ExponentialFactory()
    estimatedDistribution = factory.build(sample)
    print("distribution=", repr(distribution))
    print("Estimated distribution=", repr(estimatedDistribution))
    estimatedDistribution = factory.build()
    print("Default distribution=", estimatedDistribution)
    estimatedDistribution = factory.build(
        distribution.getParametersCollection())
    print("Distribution from parameters=", estimatedDistribution)
    estimatedExponential = factory.buildAsExponential(sample)
    print("Exponential          =", distribution)
    print("Estimated exponential=", estimatedExponential)
    estimatedExponential = factory.buildAsExponential()
    print("Default exponential=", estimatedExponential)
    estimatedExponential = factory.buildAsExponential(
        distribution.getParametersCollection())
    print("Exponential from parameters=", estimatedExponential)

except:
    import sys
    print("t_ExponentialFactory_std.py", sys.exc_info()[0], sys.exc_info()[1])