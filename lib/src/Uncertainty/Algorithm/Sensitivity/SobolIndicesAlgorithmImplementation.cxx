//                                               -*- C++ -*-
/**
 *  @brief Implementation for sensitivity algorithms
 *
 *  Copyright 2005-2018 Airbus-EDF-IMACS-Phimeca
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
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "openturns/SobolIndicesAlgorithmImplementation.hxx"
#include "openturns/PersistentObjectFactory.hxx"
#include "openturns/ResourceMap.hxx"
#include "openturns/RandomGenerator.hxx"
#include "openturns/TBB.hxx"
#include "openturns/Cloud.hxx"
#include "openturns/Curve.hxx"
#include "openturns/Pie.hxx"
#include "openturns/Text.hxx"
#include "openturns/SobolIndicesExperiment.hxx"
#include "openturns/Normal.hxx"
#include "openturns/KernelSmoothing.hxx"

BEGIN_NAMESPACE_OPENTURNS

CLASSNAMEINIT(SobolIndicesAlgorithmImplementation)

static const Factory<SobolIndicesAlgorithmImplementation> Factory_SobolIndicesAlgorithmImplementation;

/* Default constructor */
SobolIndicesAlgorithmImplementation::SobolIndicesAlgorithmImplementation()
  : PersistentObject()
  , inputDesign_()
  , outputDesign_()
  , size_(0)
  , bootstrapSize_(ResourceMap::GetAsUnsignedInteger("SobolIndicesAlgorithm-DefaultBootstrapSize"))
  , confidenceLevel_(ResourceMap::GetAsScalar("SobolIndicesAlgorithm-DefaultBootstrapConfidenceLevel"))
  , referenceVariance_()
  , varianceI_()
  , varianceTI_()
  , mergedFirstOrderIndices_()
  , mergedTotalOrderIndices_()
  , secondOrderIndices_()
  , firstOrderIndiceDistribution_()
  , totalOrderIndiceDistribution_()
  , alreadyComputedIndicesDistribution_(false)
  , useAsymptoticDistribution_(ResourceMap::GetAsBool("SobolIndicesAlgorithm-DefaultUseAsymptoticDistribution"))
{
  // Nothing to do
}

/** Constructor with parameters */
SobolIndicesAlgorithmImplementation::SobolIndicesAlgorithmImplementation(const Sample & inputDesign,
    const Sample & outputDesign,
    const UnsignedInteger size)
  : PersistentObject()
  , inputDesign_(inputDesign)
  , outputDesign_(outputDesign)
  , size_(size)
  , bootstrapSize_(ResourceMap::GetAsUnsignedInteger("SobolIndicesAlgorithm-DefaultBootstrapSize"))
  , confidenceLevel_(ResourceMap::GetAsScalar("SobolIndicesAlgorithm-DefaultBootstrapConfidenceLevel"))
  , referenceVariance_()
  , varianceI_()
  , varianceTI_()
  , mergedFirstOrderIndices_()
  , mergedTotalOrderIndices_()
  , secondOrderIndices_()
  , firstOrderIndiceDistribution_()
  , totalOrderIndiceDistribution_()
  , alreadyComputedIndicesDistribution_(false)
  , useAsymptoticDistribution_(ResourceMap::GetAsBool("SobolIndicesAlgorithm-DefaultUseAsymptoticDistribution"))
{
  if (outputDesign.getSize() == 0)
    throw InvalidArgumentException(HERE) << "In SobolIndicesAlgorithmImplementation::SobolIndicesAlgorithmImplementation, output design is empty" ;

  // center Y
  Point muY(outputDesign_.computeMean());
  outputDesign_ -= muY;

  // Check if desing result is coherant
  if (inputDesign.getSize() != outputDesign.getSize())
    throw InvalidArgumentException(HERE) << "In SobolIndicesAlgorithmImplementation::SobolIndicesAlgorithmImplementation, input and output designs have different size. Input design size=" << inputDesign.getSize()
                                         << ", wheras output design size=" << outputDesign.getSize();
  // Reference sample and its variance
  Sample outReference(size, outputDesign.getDimension());
  for (UnsignedInteger k = 0; k < size; ++k) outReference[k] = outputDesign[k];
  referenceVariance_ = outReference.computeVariance();
}

/* Virtual constructor */
SobolIndicesAlgorithmImplementation * SobolIndicesAlgorithmImplementation::clone() const
{
  return new SobolIndicesAlgorithmImplementation(*this);
}

/** Constructor with distribution / model parameters */
SobolIndicesAlgorithmImplementation::SobolIndicesAlgorithmImplementation(const Distribution & distribution,
    const UnsignedInteger size,
    const Function & model,
    const Bool computeSecondOrder)
  : PersistentObject()
  , inputDesign_()
  , outputDesign_()
  , size_(size)
  , bootstrapSize_(ResourceMap::GetAsUnsignedInteger("SobolIndicesAlgorithm-DefaultBootstrapSize"))
  , confidenceLevel_(ResourceMap::GetAsScalar("SobolIndicesAlgorithm-DefaultBootstrapConfidenceLevel"))
  , referenceVariance_()
  , varianceI_()
  , varianceTI_()
  , mergedFirstOrderIndices_()
  , mergedTotalOrderIndices_()
  , secondOrderIndices_()
  , firstOrderIndiceDistribution_()
  , totalOrderIndiceDistribution_()
  , alreadyComputedIndicesDistribution_(false)
  , useAsymptoticDistribution_(ResourceMap::GetAsBool("SobolIndicesAlgorithm-DefaultUseAsymptoticDistribution"))
{
  const UnsignedInteger inputDimension = model.getInputDimension();
  if (inputDimension != distribution.getDimension())
    throw InvalidArgumentException(HERE) << "In SobolIndicesAlgorithmImplementation::SobolIndicesAlgorithmImplementation, incompatible dimension between model and distribution. distribution dimension=" << distribution.getDimension()
                                         << ", model input dimension = " << inputDimension;
  const SobolIndicesExperiment sobolExperiment(distribution, size, computeSecondOrder);
  inputDesign_ = sobolExperiment.generate();
  outputDesign_ = model(inputDesign_);

  if (computeSecondOrder && (inputDimension == 2))
  {
    // Special case: the experiment does not contain the C=[E_2, E_1] sample
    Sample E1(outputDesign_, size * 2, size * 3);
    Sample E2(outputDesign_, size * 3, size * 4);
    outputDesign_.add(E2);
    outputDesign_.add(E1);
  }

  // center Y
  Point muY(outputDesign_.computeMean());
  outputDesign_ -= muY;

  // Reference sample and its variance
  Sample outReference(size, outputDesign_.getDimension());
  for (UnsignedInteger k = 0; k < size_; ++k) outReference[k] = outputDesign_[k];
  referenceVariance_ = outReference.computeVariance();
}

/** Constructor with distribution / model parameters */
SobolIndicesAlgorithmImplementation::SobolIndicesAlgorithmImplementation(const WeightedExperiment & experiment,
    const Function & model,
    const Bool computeSecondOrder)
  : PersistentObject()
  , inputDesign_()
  , outputDesign_()
  , size_(experiment.getSize())
  , bootstrapSize_(ResourceMap::GetAsUnsignedInteger("SobolIndicesAlgorithm-DefaultBootstrapSize"))
  , confidenceLevel_(ResourceMap::GetAsScalar("SobolIndicesAlgorithm-DefaultBootstrapConfidenceLevel"))
  , referenceVariance_()
  , varianceI_()
  , varianceTI_()
  , mergedFirstOrderIndices_()
  , mergedTotalOrderIndices_()
  , secondOrderIndices_()
  , firstOrderIndiceDistribution_()
  , totalOrderIndiceDistribution_()
  , alreadyComputedIndicesDistribution_(false)
  , useAsymptoticDistribution_(ResourceMap::GetAsBool("SobolIndicesAlgorithm-DefaultUseAsymptoticDistribution"))
{
  const UnsignedInteger inputDimension = model.getInputDimension();
  if (inputDimension != experiment.getDistribution().getDimension())
    throw InvalidArgumentException(HERE) << "In SobolIndicesAlgorithmImplementation::SobolIndicesAlgorithmImplementation, incompatible dimension between model and distribution. Experiment dimension=" << experiment.getDistribution().getDimension()
                                         << ", model input dimension = " << inputDimension;
  const SobolIndicesExperiment sobolExperiment(experiment, computeSecondOrder);
  inputDesign_ = sobolExperiment.generate();
  outputDesign_ = model(inputDesign_);

  // center Y
  Point muY(outputDesign_.computeMean());
  outputDesign_ -= muY;

  // Reference sample and its variance
  Sample outReference(size_, outputDesign_.getDimension());
  for (UnsignedInteger k = 0; k < size_; ++k) outReference[k] = outputDesign_[k];
  referenceVariance_ = outReference.computeVariance();
}


/* First order indices accessor */
Point SobolIndicesAlgorithmImplementation::getFirstOrderIndices(const UnsignedInteger marginalIndex) const
{
  if (0 == varianceI_.getSize())
  {
    // Invoke internal method to compute first/ total order indices indices
    // This method is defined in specific children classes
    varianceI_ = computeIndices(outputDesign_, varianceTI_);
  }
  const UnsignedInteger outputDimension = outputDesign_.getDimension();
  if (marginalIndex >= outputDimension)
    throw InvalidArgumentException(HERE) << "In SobolIndicesAlgorithmImplementation::getTotalOrderIndices, marginalIndex should be in [0," << outputDimension - 1;
  // return value
  const Point firstOrderSensitivity(varianceI_[marginalIndex] / referenceVariance_[marginalIndex]);
  for (UnsignedInteger p = 0; p < inputDesign_.getDimension(); ++p)
  {
    if ((firstOrderSensitivity[p] > 1.0) || firstOrderSensitivity[p] < 0.0)
      LOGWARN(OSS() << "The estimated first order Sobol index (" << p << ") is not in the range [0, 1]. You may increase the sampling size. HERE we have: S_"
              << p << "=" <<  firstOrderSensitivity << ", ST_" << p << "=" << varianceTI_(marginalIndex, p) / referenceVariance_[marginalIndex]);
    // Another case : Si > STi
    if (varianceI_(marginalIndex, p) > varianceTI_(marginalIndex, p))
      LOGWARN(OSS() << "The estimated first order Sobol index (" << p << ") is greater than its total order index . You may increase the sampling size. HERE we have: S_"
              << p << "=" <<  firstOrderSensitivity << ", ST_" << p << "=" << varianceTI_(marginalIndex, p) / referenceVariance_[marginalIndex]);
  }
  return firstOrderSensitivity;
}

struct BootstrapPolicy
{
  const SobolIndicesAlgorithmImplementation & sai_;
  const Indices & indices_;
  const UnsignedInteger size_;
  Sample & bsFO_;
  Sample & bsTO_;

  BootstrapPolicy( const SobolIndicesAlgorithmImplementation & sai,
                   const Indices & indices,
                   const UnsignedInteger size,
                   Sample & bsFO,
                   Sample & bsTO)
    : sai_(sai)
    , indices_(indices)
    , size_(size)
    , bsFO_(bsFO)
    , bsTO_(bsTO)
  {}

  inline void operator()( const TBB::BlockedRange<UnsignedInteger> & r ) const
  {
    Indices slice(size_);
    Sample VTi;
    Point mergedTotal;

    for (UnsignedInteger k = r.begin(); k != r.end(); ++k)
    {
      // Extract indices
      memcpy(&slice[0], &indices_[k * size_], size_ * sizeof(UnsignedInteger));

      // Generate huge random sample using Bootstrap algorithm
      const Sample randomCollection(sai_.getBootstrapDesign(slice));
      // Pseudo-Reference variance
      Sample outReference(size_, sai_.outputDesign_.getDimension());
      for (UnsignedInteger i = 0; i < size_; ++i) outReference[i] = randomCollection[i];
      const Point variance(outReference.computeVariance());
      // Compute indices using this collection
      const Sample Vi(sai_.computeIndices(randomCollection, VTi));
      // Compute aggregated indices
      bsFO_[k] = sai_.computeAggregatedIndices(Vi, VTi, variance, mergedTotal);
      // Add to sample
      bsTO_[k] = mergedTotal;
    }
  }
}; /* end struct BootstrapPolicy */

/** void method that computes confidence interval */
void SobolIndicesAlgorithmImplementation::computeBootstrapDistribution() const
{
  // Build interval using sample variance
  // Mean reference is the Sensitivity values
  const Point aggregatedFirstOrder(getAggregatedFirstOrderIndices());
  const Point aggregatedTotalOrder(getAggregatedTotalOrderIndices());
  if (bootstrapSize_ > 0)
  {
    // Temporary samples that stores the first/total indices
    const UnsignedInteger inputDimension = inputDesign_.getDimension();
    Sample bsFO(0, inputDimension);
    Sample bsTO(0, inputDimension);
    const UnsignedInteger size = size_;
    // To have the exact same results with TBB, we have to precompute
    // RandomGenerator::IntegerGenerate calls and store results in a
    // variable.  This would require lots of memory when bootstrapSize_ is
    // large, thus let user specify block size.
    const UnsignedInteger blockSize = std::min(bootstrapSize_, ResourceMap::GetAsUnsignedInteger("SobolIndicesAlgorithm-DefaultBlockSize"));
    const UnsignedInteger maximumOuterSampling = bootstrapSize_ / blockSize;
    const UnsignedInteger modulo = bootstrapSize_ % blockSize;
    const UnsignedInteger lastBlockSize = modulo == 0 ? blockSize : modulo;

    for(UnsignedInteger outerSampling = 0; outerSampling < maximumOuterSampling; ++outerSampling)
    {
      // the last block can be smaller
      const UnsignedInteger effectiveBlockSize = (outerSampling + 1) < maximumOuterSampling ? blockSize : lastBlockSize;

      Sample bsFOpartial(effectiveBlockSize, inputDimension);
      Sample bsTOpartial(effectiveBlockSize, inputDimension);
      const RandomGenerator::UnsignedIntegerCollection randomIndices(RandomGenerator::IntegerGenerate(size * effectiveBlockSize, size));
      const Indices indices(randomIndices.begin(), randomIndices.end());
      const BootstrapPolicy policy( *this, indices, size, bsFOpartial, bsTOpartial );
      TBB::ParallelFor( 0, effectiveBlockSize, policy );

      bsFO.add(bsFOpartial);
      bsTO.add(bsTOpartial);
    }

    KernelSmoothing factory;
    firstOrderIndiceDistribution_ = factory.build(bsFO);
    totalOrderIndiceDistribution_ = factory.build(bsTO);
  }
}


void SobolIndicesAlgorithmImplementation::computeAsymptoticDistribution() const
{
  throw NotYetImplementedException(HERE) << "SobolIndicesAlgorithmImplementation::computeAsymptoticInterval";
}


Scalar SobolIndicesAlgorithmImplementation::computeVariance(const Sample & u, const Function & psi) const
{
  Point gradient(*psi.gradient(u.computeMean()).getImplementation());
  return dot(gradient, u.computeCovariance() * gradient) / size_;
}

void SobolIndicesAlgorithmImplementation::setConfidenceInterval(const Point & varianceFO,
    const Point & varianceTO) const
{
  const UnsignedInteger inputDimension = inputDesign_.getDimension();
  Point stdandardDeviationFO(inputDimension);
  Point stdandardDeviationTO(inputDimension);
  for (UnsignedInteger p = 0; p < inputDimension; ++ p)
  {
    stdandardDeviationFO[p] = sqrt(varianceFO[p]);
    stdandardDeviationTO[p] = sqrt(varianceTO[p]);
  }
  const Point aggregatedFO(getAggregatedFirstOrderIndices());
  const Point aggregatedTO(getAggregatedTotalOrderIndices());
  firstOrderIndiceDistribution_ = Normal(aggregatedFO, stdandardDeviationFO, CorrelationMatrix(inputDimension));
  totalOrderIndiceDistribution_ = Normal(aggregatedTO, stdandardDeviationTO, CorrelationMatrix(inputDimension));
}


/* Interval for the first order indices accessor */
Interval SobolIndicesAlgorithmImplementation::getFirstOrderIndicesInterval() const
{
  const Distribution distribution = getFirstOrderIndicesDistribution();
  const UnsignedInteger inputDimension = distribution.getDimension();
  Point lowerBound(inputDimension);
  Point upperBound(inputDimension);
  for (UnsignedInteger j = 0; j < inputDimension; ++ j)
  {
    Distribution marginal(distribution.getMarginal(j));
    lowerBound[j] = marginal.computeQuantile(0.5 * (1.0 - confidenceLevel_))[0];
    upperBound[j] = marginal.computeQuantile(0.5 * (1.0 + confidenceLevel_))[0];
  }
  return Interval(lowerBound, upperBound);
}

/* Second order indices accessor */
SymmetricMatrix SobolIndicesAlgorithmImplementation::getSecondOrderIndices(const UnsignedInteger marginalIndex) const
{
  const UnsignedInteger outputDimension = outputDesign_.getDimension();
  if (0 == secondOrderIndices_.getNbSheets())
  {
    const UnsignedInteger inputDimension = inputDesign_.getDimension();
    // Check if is possible
    if (outputDesign_.getSize() < 2 * (inputDesign_.getDimension() + 1) * size_ )
      throw InvalidArgumentException(HERE) << "In SobolIndicesAlgorithmImplementation::getSecondOrderIndices, second order indices designs not computed";
    // Compute second order indices
    secondOrderIndices_ = SymmetricTensor(inputDimension, outputDimension);
    // Compute cross square mean between samples yA and yB, which are located respectively at index 0 and size_
    Point crossSquareMean(computeSumDotSamples(outputDesign_, size_, 0, size_) / size_);

    // Main loop
    for (UnsignedInteger k1 = 0; k1 < inputDimension; ++k1)
    {
      for (UnsignedInteger k2 = 0; k2 < k1; ++k2)
      {
        // Compute yEDotyC
        //const Point yEDotyC(computeSumDotSamples(yE, yC));
        const Point yEDotyC(computeSumDotSamples(outputDesign_, size_, (2 + k1) * size_, (2 + k2 + inputDimension) * size_));
        for (UnsignedInteger q = 0; q < outputDimension; ++q)
        {
          // Sij = (Vij - crossMean)/var - and S_{i}, S_{j}
          const Point firstOrderIndices(getFirstOrderIndices(q));
          secondOrderIndices_(k1, k2, q) = (yEDotyC[q] / (size_ - 1.0) - crossSquareMean[q]) / referenceVariance_[q] - firstOrderIndices[k1] - firstOrderIndices[k2] ;
          if ((secondOrderIndices_(k1, k2, q) < 0.0) || (secondOrderIndices_(k1, k2, q) > 1.0))
            LOGWARN(OSS() << "The estimated second order Sobol index (" << k1 << ", " << k2 << ") is not in the range [0, 1]. You may increase the sampling size.");
        }
      }
    }
  }
  if (marginalIndex >= outputDimension)
    throw InvalidArgumentException(HERE) << "In SobolIndicesAlgorithmImplementation::getSecondOrderIndices, marginalIndex should be in [0," << outputDimension - 1 << "]";
  return secondOrderIndices_.getSheet(marginalIndex);
}

/* Total order indices accessor */
Point SobolIndicesAlgorithmImplementation::getTotalOrderIndices(const UnsignedInteger marginalIndex) const
{
  if (0 == varianceI_.getSize())
  {
    // Invoke internal method to compute first/ total order indices indices
    // This method is defined in specific children classes
    varianceI_ = computeIndices(outputDesign_, varianceTI_);
  }
  const UnsignedInteger outputDimension = outputDesign_.getDimension();
  const UnsignedInteger inputDimension = inputDesign_.getDimension();
  if (marginalIndex >= outputDimension)
    throw InvalidArgumentException(HERE) << "In SobolIndicesAlgorithmImplementation::getTotalOrderIndices, marginalIndex should be in [0," << outputDimension - 1;
  for (UnsignedInteger p = 0; p < inputDimension; ++p)
  {
    // Another case : Si > STi
    if (varianceI_(marginalIndex, p) > varianceTI_(marginalIndex, p))
      LOGWARN(OSS() << "The estimated total order Sobol index (" << p << ") is lesser than first order index . You may increase the sampling size. HERE we have: S_"
              << p << "=" <<  varianceI_(marginalIndex, p) / referenceVariance_[marginalIndex] << ", ST_" << p << "=" << varianceTI_(marginalIndex, p) / referenceVariance_[marginalIndex]);
  }
  // return value
  return varianceTI_[marginalIndex] / referenceVariance_[marginalIndex] ;
}

/* Interval for the total order indices accessor */
Interval SobolIndicesAlgorithmImplementation::getTotalOrderIndicesInterval() const
{
  const Distribution distribution = getTotalOrderIndicesDistribution();
  const UnsignedInteger inputDimension = distribution.getDimension();
  Point lowerBound(inputDimension);
  Point upperBound(inputDimension);
  for (UnsignedInteger j = 0; j < inputDimension; ++ j)
  {
    Distribution marginal(distribution.getMarginal(j));
    lowerBound[j] = marginal.computeQuantile(0.5 * (1.0 - confidenceLevel_))[0];
    upperBound[j] = marginal.computeQuantile(0.5 * (1.0 + confidenceLevel_))[0];
  }
  return Interval(lowerBound, upperBound);
}

void SobolIndicesAlgorithmImplementation::computeIndicesDistribution() const
{
  // if not already computed
  if (!alreadyComputedIndicesDistribution_)
  {
    if (useAsymptoticDistribution_)
    {
      computeAsymptoticDistribution();
    }
    else
    {
      computeBootstrapDistribution();
    }
    alreadyComputedIndicesDistribution_ = true;
  }
}


Distribution SobolIndicesAlgorithmImplementation::getFirstOrderIndicesDistribution() const
{
  computeIndicesDistribution();
  return firstOrderIndiceDistribution_;
}

Distribution SobolIndicesAlgorithmImplementation::getTotalOrderIndicesDistribution() const
{
  computeIndicesDistribution();
  return totalOrderIndiceDistribution_;
}

/* Aggregated first order indices accessor for multivariate samples */
Point SobolIndicesAlgorithmImplementation::getAggregatedFirstOrderIndices() const
{
  if (0 == varianceI_.getSize())
  {
    // Invoke internal method to compute first/ total order indices indices
    varianceI_ = computeIndices(outputDesign_, varianceTI_);
  }
  if (0 == mergedFirstOrderIndices_.getDimension())
  {
    // Compute aggregate indices
    mergedFirstOrderIndices_ = computeAggregatedIndices(varianceI_, varianceTI_, referenceVariance_, mergedTotalOrderIndices_);
  }
  // Indices computed
  return mergedFirstOrderIndices_;
}

/* Aggregated total order indices accessor for multivariate samples */
Point SobolIndicesAlgorithmImplementation::getAggregatedTotalOrderIndices() const
{
  if (0 == varianceI_.getSize())
  {
    // Invoke internal method to compute first/ total order indices indices
    varianceI_ = computeIndices(outputDesign_, varianceTI_);
  }
  if (0 == mergedFirstOrderIndices_.getDimension())
  {
    // Compute aggregate indices
    mergedFirstOrderIndices_ = computeAggregatedIndices(varianceI_, varianceTI_, referenceVariance_, mergedTotalOrderIndices_);
  }
  // Indices computed
  return mergedTotalOrderIndices_;
}

// Getter for bootstrap size
UnsignedInteger SobolIndicesAlgorithmImplementation::getBootstrapSize() const
{
  return bootstrapSize_;
}

// Setter for bootstrap size
void SobolIndicesAlgorithmImplementation::setBootstrapSize(const UnsignedInteger bootstrapSize)
{
  if (bootstrapSize <= 0)
    throw InvalidArgumentException(HERE) << "Bootstrap sampling size should be positive. Here, bootstrapSize=" << bootstrapSize;
  bootstrapSize_ = bootstrapSize;
}

// Getter for bootstrap confidence level
Scalar SobolIndicesAlgorithmImplementation::getConfidenceLevel() const
{
  return confidenceLevel_;
}

// Setter for bootstrap confidence level
void SobolIndicesAlgorithmImplementation::setConfidenceLevel(const Scalar confidenceLevel)
{
  if ((confidenceLevel < 0.0) || (confidenceLevel >= 1.0))
    throw InvalidArgumentException(HERE) << "Confidence level value should be in ]0,1[. Here, confidence level=" << confidenceLevel;
  confidenceLevel_ = confidenceLevel;
}

/* String converter */
String SobolIndicesAlgorithmImplementation::__repr__() const
{
  OSS oss;
  oss << "class=" << GetClassName()
      << " name=" << getName();
  return oss;
}

// Multiplication and sum of two Samples
// TODO Write method in Sample ?
Point SobolIndicesAlgorithmImplementation::computeSumDotSamples(const Sample & x,
    const Sample & y) const
{
  // Internal method
  // Suppose that samples have the same size, same dimension
  const UnsignedInteger dimension = x.getDimension();
  const UnsignedInteger size = x.getSize();

  const Scalar * xptr(&x(0, 0));
  const Scalar * yptr(&y(0, 0));


  Point value(dimension, 0.0);
  for (UnsignedInteger i = 0; i < size; ++i)
    for (UnsignedInteger j = 0; j < dimension; ++j, ++xptr, ++yptr)
      value[j] += (*xptr) * (*yptr);
  return value;
}

// Multiplication and sum of two Samples, contained in the sampe sample
Point SobolIndicesAlgorithmImplementation::computeSumDotSamples(const Sample & sample,
    const UnsignedInteger size,
    const UnsignedInteger indexX,
    const UnsignedInteger indexY) const
{
  // Internal method
  // Suppose that samples have the same size, same dimension
  const UnsignedInteger dimension = sample.getDimension();

  const Scalar * xptr(&sample(indexX, 0));
  const Scalar * yptr(&sample(indexY, 0));


  Point value(dimension, 0.0);
  for (UnsignedInteger i = 0; i < size; ++i)
    for (UnsignedInteger j = 0; j < dimension; ++j, ++xptr, ++yptr)
      value[j] += (*xptr) * (*yptr);
  return value;
}

/* Multiplication of two sub-samples */
Sample SobolIndicesAlgorithmImplementation::ComputeProdSample(const Sample & sample,
    const UnsignedInteger marginalIndex,
    const UnsignedInteger size,
    const UnsignedInteger indexX,
    const UnsignedInteger indexY)
{
  Sample prod(size, 1);
  for (UnsignedInteger i = 0; i < size; ++ i)
    prod(i, 0) = sample(i + indexX, marginalIndex) * sample(i + indexY, marginalIndex);
  return prod;
}

/* String converter */
String SobolIndicesAlgorithmImplementation::__str__(const String & ) const
{
  return __repr__();
}

/** Internal method that compute Vi/VTi using a collection of samples */
Sample SobolIndicesAlgorithmImplementation::computeIndices(const Sample & ,
    Sample & ) const
{
  // Method is defined in Jansan/Saltelli/Martinez/Mauntz classes
  throw new NotYetImplementedException(HERE);
}

/** Method that draw (plot) the sensitivity graph */
Graph SobolIndicesAlgorithmImplementation::draw() const
{
  Graph graph(OSS() << " Aggregated sensitivity indices - " << getClassName(), "inputs", "Sensitivity indices ", true, "");
  // Define cloud for first order and total order indices
  const Point aggregatedFO(getAggregatedFirstOrderIndices());
  const Point aggregatedTO(getAggregatedTotalOrderIndices());
  const Description inputDescription(inputDesign_.getDescription());
  const UnsignedInteger dimension = aggregatedFO.getDimension();
  Sample data(dimension, 2);
  for (UnsignedInteger k = 0; k < dimension; ++k)
  {
    data(k, 0) = k + 1;
    data(k, 1) = aggregatedFO[k];
  }
  // Define cloud for FO
  Cloud firstOrderIndicesGraph(data, "red", "circle", "Aggregated FO");
  graph.add(firstOrderIndicesGraph);
  // Total order
  for (UnsignedInteger k = 0; k < dimension; ++k)
  {
    data(k, 0) = (k + 1) + 0.1;
    data(k, 1) = aggregatedTO[k];
  }
  // Define cloud for TO
  Cloud totalOrderIndicesGraph(data, "blue", "square", "Aggregated TO");
  graph.add(totalOrderIndicesGraph);
  // Description
  for (UnsignedInteger k = 0; k < dimension; ++k)
  {
    data(k, 0) = (k + 1) + 0.2;
    data(k, 1) = 0.5 * (aggregatedTO[k] + aggregatedFO[k]);
  }
  Text text(data, inputDescription, "right");
  text.setColor("black");
  graph.add(text);
  // Set bounding box
  Point lowerBound(2, -0.1);
  Point upperBound(2);
  upperBound[0] = dimension + 1.0;
  upperBound[1] = 1.1;
  if (bootstrapSize_ > 0 && confidenceLevel_ > 0.0)
  {
    // Add plot of intervals
    const Interval foInterval(getFirstOrderIndicesInterval());
    const Interval toInterval(getTotalOrderIndicesInterval());
    // transform data
    data = Sample(2, 2);
    for (UnsignedInteger k = 0; k < dimension; ++k)
    {
      // Relative to FirstOrder
      data(0, 0) = (k + 1);
      data(0, 1) = foInterval.getLowerBound()[k];
      data(1, 0) = (k + 1);
      data(1, 1) = foInterval.getUpperBound()[k];
      graph.add(Curve(data, "red", "solid", 2, ""));

      // Relative to TotalOrder
      data(0, 0) = (k + 1) + 0.1;
      data(0, 1) = toInterval.getLowerBound()[k];
      data(1, 0) = (k + 1) + 0.1;
      data(1, 1) = toInterval.getUpperBound()[k];
      graph.add(Curve(data, "blue", "solid", 2, ""));
    }
  }
  graph.setBoundingBox(Interval(lowerBound, upperBound));
  graph.setLegendPosition("topright");
  return graph;
}

/** Method that draw the sensitivity graph of a fixed marginal */
Graph SobolIndicesAlgorithmImplementation::draw(UnsignedInteger marginalIndex) const
{
  Graph graph(OSS() << " Sensitivity indices - " << getClassName(), "inputs", "Sensitivity indices ", true, "");
  // Define cloud for first order and total order indices
  const Point foIndices(getFirstOrderIndices(marginalIndex));
  const Point toIndices(getTotalOrderIndices(marginalIndex));
  Sample data(foIndices.getDimension(), 2);
  for (UnsignedInteger k = 0; k < foIndices.getDimension(); ++k)
  {
    data(k, 0) = k + 1;
    data(k, 1) = foIndices[k];
  }
  // Define cloud for FO
  Cloud firstOrderIndicesGraph(data, "red", "circle", "Aggregated FO");
  graph.add(firstOrderIndicesGraph);
  // Total order
  for (UnsignedInteger k = 0; k < foIndices.getDimension(); ++k)
  {
    data(k, 0) = (k + 1) + 0.1;
    data(k, 1) = toIndices[k];
  }
  // Define cloud for TO
  Cloud totalOrderIndicesGraph(data, "blue", "square", "Aggregated TO");
  graph.add(totalOrderIndicesGraph);
  // Set bounding box
  Point lowerBound(2, -0.1);
  Point upperBound(2);
  upperBound[0] = foIndices.getDimension() + 1;
  upperBound[1] = 1.1 ;
  graph.setBoundingBox(Interval(lowerBound, upperBound));
  graph.setLegendPosition("topright");
  return graph;
}

/** Internal method that returns a bootstrap NSC */
Sample SobolIndicesAlgorithmImplementation::getBootstrapDesign(const Indices & indices) const
{
  // Bootstrap with huge sample that contains several samples
  const UnsignedInteger inputDimension = inputDesign_.getDimension();
  const UnsignedInteger outputDimension = outputDesign_.getDimension();
  Sample bootstrapDesign(0, outputDimension);
  for (UnsignedInteger p = 0; p < 2 + inputDimension; ++p)
  {
    Sample y(size_, outputDimension);
    Scalar* yPermData = &y(0, 0);

    const Scalar* yData = &outputDesign_(p * size_, 0);
    for (UnsignedInteger k = 0; k < size_; ++k, yPermData += outputDimension)
      memcpy(yPermData, &yData[indices[k] * outputDimension], outputDimension * sizeof(Scalar));
    // add samples to the collection
    bootstrapDesign.add(y);
  }
  return bootstrapDesign;
}

/** Function that computes merged indices using Vi/VTi + variance  */
Point SobolIndicesAlgorithmImplementation::computeAggregatedIndices(const Sample & Vi,
    const Sample & VTi,
    const Point & variance,
    Point & mergedTotal) const
{
  // Generic implementation
  const UnsignedInteger inputDimension = Vi.getDimension();
  const UnsignedInteger outputDimension = Vi.getSize();
  if (inputDimension == 1)
  {
    mergedTotal = Point(VTi[0]);
    return Point(Vi[0]);
  }

  // Compute sum of Var(Y^k)
  Scalar sumVariance = variance.norm1();

  // Compute merged indices
  mergedTotal = VTi.computeMean() * (outputDimension / sumVariance);
  return Point(Vi.computeMean() * (outputDimension / sumVariance));
}

/* ImportanceFactors graph */
Graph SobolIndicesAlgorithmImplementation::DrawImportanceFactors(const PointWithDescription & importanceFactors,
    const String & title)
{
  return DrawImportanceFactors(importanceFactors, importanceFactors.getDescription(), title);
}

/* ImportanceFactors graph */
Graph SobolIndicesAlgorithmImplementation::DrawImportanceFactors(const Point & values,
    const Description & names,
    const String & title)
{
  /* build data for the pie */
  const UnsignedInteger dimension = values.getDimension();
  if (dimension == 0) throw InvalidArgumentException(HERE) << "Error: cannot draw an importance factors pie based on empty data.";
  if ((names.getSize() != 0) && (names.getSize() != dimension)) throw InvalidArgumentException(HERE) << "Error: the names size must match the value dimension.";
  Scalar l1Norm = 0.0;
  for (UnsignedInteger i = 0; i < dimension; ++i) l1Norm += std::abs(values[i]);
  if (l1Norm == 0.0) throw InvalidArgumentException(HERE) << "Error: cannot draw an importance factors pie based on null data.";
  Point data(dimension);
  /* Normalization */
  for (UnsignedInteger i = 0; i < dimension; ++i) data[i] = values[i] / l1Norm;
  /* we build the pie */
  Pie importanceFactorsPie(data);

  /* build labels and colors for the pie */
  Description palette(dimension);
  Description labels(dimension);
  Description description(names);
  // If no description has been given for the input distribution components, give standard ones
  if (description.getSize() != dimension)
  {
    description = Description(dimension);
    for (UnsignedInteger i = 0; i < dimension; ++i) description[i] = String(OSS() << "Component " << i);
  }
  for (UnsignedInteger i = 0; i < dimension; ++i)
  {
    OSS oss(false);
    oss << description[i] << " : " << std::fixed;
    oss.setPrecision(1);
    oss << 100.0 * data[i] << "%";
    labels[i] = oss;
  }
  /* we complete the pie */
  importanceFactorsPie.setLabels(labels);
  importanceFactorsPie.buildDefaultPalette();
  /* we build the graph with a title */
  Graph importanceFactorsGraph(title);
  /* we embed the pie into the graph */
  importanceFactorsGraph.add(importanceFactorsPie);
  return importanceFactorsGraph;
}

void SobolIndicesAlgorithmImplementation::setUseAsymptoticDistribution(Bool useAsymptoticDistribution)
{
  if (useAsymptoticDistribution_ != useAsymptoticDistribution)
  {
    useAsymptoticDistribution_ = useAsymptoticDistribution;
    alreadyComputedIndicesDistribution_ = false;
  }
}

Bool SobolIndicesAlgorithmImplementation::getUseAsymptoticDistribution() const
{
  return useAsymptoticDistribution_;
}

/* Method save() stores the object through the StorageManager */
void SobolIndicesAlgorithmImplementation::save(Advocate & adv) const
{
  PersistentObject::save(adv);
  adv.saveAttribute( "inputDesign_", inputDesign_ );
  adv.saveAttribute( "outputDesign_", outputDesign_ );
  adv.saveAttribute( "size_", size_ );
  adv.saveAttribute( "bootstrapSize_", bootstrapSize_ );
  adv.saveAttribute( "confidenceLevel_", confidenceLevel_ );
  adv.saveAttribute( "referenceVariance_",  referenceVariance_);
  adv.saveAttribute( "varianceI_", varianceI_);
  adv.saveAttribute( "varianceTI_", varianceTI_);
  adv.saveAttribute( "mergedFirstOrderIndices_", mergedFirstOrderIndices_);
  adv.saveAttribute( "mergedTotalOrderIndices_", mergedTotalOrderIndices_);
  adv.saveAttribute( "secondOrderIndices_", secondOrderIndices_);
  adv.saveAttribute( "firstOrderIndiceDistribution_", firstOrderIndiceDistribution_);
  adv.saveAttribute( "totalOrderIndiceDistribution_", totalOrderIndiceDistribution_);
  adv.saveAttribute( "alreadyComputedIndicesDistribution_", alreadyComputedIndicesDistribution_);
  adv.saveAttribute( "useAsymptoticDistribution_" , useAsymptoticDistribution_);
}

/* Method load() reloads the object from the StorageManager */
void SobolIndicesAlgorithmImplementation::load(Advocate & adv)
{
  PersistentObject::load(adv);
  adv.loadAttribute( "inputDesign_", inputDesign_ );
  adv.loadAttribute( "outputDesign_", outputDesign_ );
  adv.loadAttribute( "size_", size_ );
  adv.loadAttribute( "bootstrapSize_", bootstrapSize_ );
  adv.loadAttribute( "confidenceLevel_", confidenceLevel_ );
  adv.loadAttribute( "referenceVariance_",  referenceVariance_);
  adv.loadAttribute( "varianceI_", varianceI_);
  adv.loadAttribute( "varianceTI_", varianceTI_);
  adv.loadAttribute( "mergedFirstOrderIndices_", mergedFirstOrderIndices_);
  adv.loadAttribute( "mergedTotalOrderIndices_", mergedTotalOrderIndices_);
  adv.loadAttribute( "secondOrderIndices_", secondOrderIndices_);
  adv.loadAttribute( "firstOrderIndiceDistribution_", firstOrderIndiceDistribution_);
  adv.loadAttribute( "totalOrderIndiceDistribution_", totalOrderIndiceDistribution_);
  adv.loadAttribute( "alreadyComputedIndicesDistribution_", alreadyComputedIndicesDistribution_);
  adv.loadAttribute( "useAsymptoticDistribution_" , useAsymptoticDistribution_);
}

END_NAMESPACE_OPENTURNS
