//                                               -*- C++ -*-
/**
 *  @brief The test file of class MaximumEntropyOrderStatisticsCopula for standard methods
 *
 *  Copyright 2005-2015 Airbus-EDF-IMACS-Phimeca
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "OT.hxx"
#include "OTtestcode.hxx"

using namespace OT;
using namespace OT::Test;

class TestObject : public MaximumEntropyOrderStatisticsCopula
{
public:
  TestObject() : MaximumEntropyOrderStatisticsCopula() {}
  virtual ~TestObject() {}
};


int main(int argc, char *argv[])
{
  TESTPREAMBLE;
  OStream fullprint(std::cout);
  setRandomGenerator();
  PlatformInfo::SetNumericalPrecision(4);

  try
  {
    // Test basic functionnalities
    checkClassWithClassName<TestObject>();

    ComposedDistribution::DistributionCollection coll;
    coll.add(Trapezoidal(-2.0, -1.1, -1.0, 1.0));
    coll.add(LogUniform(1.0, 1.2));
    coll.add(Triangular(3.0, 4.0, 5.0));
    coll.add(Arcsine(3.5, 5.5));

    // Instanciate one distribution object
    MaximumEntropyOrderStatisticsCopula distribution(coll);
    UnsignedInteger dim = distribution.getDimension();
    fullprint << "Distribution " << distribution << std::endl;

    // Is this distribution elliptical ?
    fullprint << "Elliptical = " << (distribution.isElliptical() ? "true" : "false") << std::endl;

    // Is this distribution continuous ?
    fullprint << "Continuous = " << (distribution.isContinuous() ? "true" : "false") << std::endl;

    // Test for realization of distribution
    NumericalPoint oneRealization = distribution.getRealization();
    fullprint << "oneRealization=" << oneRealization << std::endl;

    // Test for sampling
    UnsignedInteger size = 10000;
    NumericalSample oneSample = distribution.getSample( size );
    fullprint << "oneSample first=" << oneSample[0] << " last=" << oneSample[size - 1] << std::endl;
    fullprint << "mean=" << oneSample.computeMean() << std::endl;
    fullprint << "covariance=" << oneSample.computeCovariance() << std::endl;
    fullprint << "skewness=" << oneSample.computeSkewness() << std::endl;
    fullprint << "kurtosis=" << oneSample.computeKurtosis() << std::endl;

    // Define a point
    NumericalPoint point(distribution.getDimension());
    for (UnsignedInteger i = 0; i < distribution.getDimension(); ++ i)
    {
      point[i] = 0.3 + i * 0.1;
    }

    fullprint << "Point= " << point << std::endl;

    // Show PDF and CDF of point
//     NumericalScalar eps(1e-5);
    NumericalPoint DDF = distribution.computeDDF( point );
    fullprint << "ddf     =" << DDF << std::endl;
//     fullprint << "ddf (FD)=" << distribution.ContinuousDistribution::computeDDF(point) << std::endl;
    NumericalScalar LPDF = distribution.computeLogPDF( point );
    fullprint << "log pdf=" << LPDF << std::endl;
    NumericalScalar PDF = distribution.computePDF( point );
    fullprint << "pdf     =" << PDF << std::endl;
    NumericalScalar CDF = distribution.computeCDF( point );
    fullprint << "cdf=" << CDF << std::endl;
    NumericalScalar CCDF = distribution.computeComplementaryCDF( point );
    fullprint << "ccdf=" << CCDF << std::endl;
//     NumericalScalar Survival = distribution.computeSurvivalFunction( point );
//     fullprint << "survival=" << Survival << std::endl;
    NumericalPoint quantile = distribution.computeQuantile( 0.95 );
    fullprint << "quantile=" << quantile << std::endl;
    fullprint << "cdf(quantile)=" << distribution.computeCDF(quantile) << std::endl;
    NumericalPoint quantileTail = distribution.computeQuantile( 0.95, true );
    fullprint << "quantile (tail)=" << quantileTail << std::endl;
    NumericalScalar CDFTail = distribution.computeComplementaryCDF( quantileTail );
    fullprint << "cdf (tail)=" << CDFTail << std::endl;
//     NumericalPoint PDFgr = distribution.computePDFGradient( point );
//     fullprint << "pdf gradient     =" << PDFgr << std::endl;
    NumericalPoint mean = distribution.getMean();
    fullprint << "mean=" << mean << std::endl;
    NumericalPoint standardDeviation = distribution.getStandardDeviation();
    fullprint << "standard deviation=" << standardDeviation << std::endl;
    NumericalPoint skewness = distribution.getSkewness();
    fullprint << "skewness=" << skewness << std::endl;
    NumericalPoint kurtosis = distribution.getKurtosis();
    fullprint << "kurtosis=" << kurtosis << std::endl;
    // CovarianceMatrix covariance = distribution.getCovariance();
    // fullprint << "covariance=" << covariance << std::endl;
    // CovarianceMatrix correlation = distribution.getCorrelation();
    // fullprint << "correlation=" << correlation << std::endl;
    // CovarianceMatrix spearman = distribution.getSpearmanCorrelation();
    // fullprint << "spearman=" << spearman << std::endl;
    //    CovarianceMatrix kendall = distribution.getKendallTau();
    //    fullprint << "kendall=" << kendall << std::endl;
    MaximumEntropyOrderStatisticsCopula::NumericalPointWithDescriptionCollection parameters = distribution.getParametersCollection();
    fullprint << "parameters=" << parameters << std::endl;
//     for (UnsignedInteger i = 0; i < 6; ++i) fullprint << "standard moment n=" << i << ", value=" << distribution.getStandardMoment(i) << std::endl;
//     fullprint << "Standard representative=" << distribution.getStandardRepresentative()->__str__() << std::endl;

    // Extract the marginals
    for (UnsignedInteger i = 0; i < dim; ++ i)
    {
      Distribution margin(distribution.getMarginal(i));
      fullprint << "margin=" << margin << std::endl;
      fullprint << "margin PDF=" << margin.computePDF(point[i]) << std::endl;
      fullprint << "margin CDF=" << margin.computeCDF(point[i]) << std::endl;
      fullprint << "margin quantile=" << margin.computeQuantile(0.95) << std::endl;
      fullprint << "margin realization=" << margin.getRealization() << std::endl;
      fullprint << "margin range=" << margin.getRange() << std::endl;
    }

    // Extract a 2-D marginal
    Indices indices(2, 0);
    indices[1] = 1;
    fullprint << "indices=" << indices << std::endl;
    Distribution margins(distribution.getMarginal(indices));
    fullprint << "margins=" << margins << std::endl;
    fullprint << "independent?=" << (margins.hasIndependentCopula() ? "true" : "false") << std::endl;
    fullprint << "margins PDF=" << margins.computePDF(NumericalPoint(2, 0.5)) << std::endl;
    fullprint << "margins CDF=" << margins.computeCDF(NumericalPoint(2, 0.5)) << std::endl;
    quantile = margins.computeQuantile(0.5);
    fullprint << "margins quantile=" << quantile << std::endl;
    fullprint << "margins CDF(quantile)=" << margins.computeCDF(quantile) << std::endl;
    fullprint << "margins realization=" << margins.getRealization() << std::endl;

  }
  catch (TestFailed & ex)
  {
    std::cerr << ex << std::endl;
    return ExitCode::Error;
  }


  return ExitCode::Success;
}