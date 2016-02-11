//                                               -*- C++ -*-
/**
 *  @brief An implementation of projection strategy as a leastSquares
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
#ifndef OPENTURNS_LEASTSQUARESSTRATEGY_HXX
#define OPENTURNS_LEASTSQUARESSTRATEGY_HXX

#include "openturns/ProjectionStrategyImplementation.hxx"
#include "openturns/PenalizedLeastSquaresAlgorithmFactory.hxx"
#include "openturns/ApproximationAlgorithmImplementationFactory.hxx"

BEGIN_NAMESPACE_OPENTURNS



/**
 * @class LeastSquaresStrategy
 *
 * An implementation of projection strategy as a leastSquares
 */

class OT_API LeastSquaresStrategy
  : public ProjectionStrategyImplementation
{
  CLASSNAME;
public:

  typedef Pointer<ApproximationAlgorithmImplementationFactory>  ApproximationAlgorithmImplementationFactoryImplementation;

  // friend class Factory<LeastSquaresStrategy>;

  /** Default constructor */
  LeastSquaresStrategy(const ApproximationAlgorithmImplementationFactory & factory = PenalizedLeastSquaresAlgorithmFactory(true));

  /** Parameter constructor */
  LeastSquaresStrategy(const Distribution & measure,
                       const ApproximationAlgorithmImplementationFactory & factory = PenalizedLeastSquaresAlgorithmFactory(true));

  /** Parameter constructor */
  LeastSquaresStrategy(const WeightedExperiment & weightedExperiment,
                       const ApproximationAlgorithmImplementationFactory & factory = PenalizedLeastSquaresAlgorithmFactory(true));

  /** Parameter constructor */
  LeastSquaresStrategy(const Distribution & measure,
                       const WeightedExperiment & weightedExperiment,
                       const ApproximationAlgorithmImplementationFactory & factory = PenalizedLeastSquaresAlgorithmFactory(true));

  /** Parameter constructor */
  LeastSquaresStrategy(const NumericalSample & inputSample,
                       const NumericalPoint & weights,
                       const NumericalSample & outputSample,
                       const ApproximationAlgorithmImplementationFactory & factory = PenalizedLeastSquaresAlgorithmFactory(true));

  LeastSquaresStrategy(const NumericalSample & inputSample,
                       const NumericalSample & outputSample,
                       const ApproximationAlgorithmImplementationFactory & factory = PenalizedLeastSquaresAlgorithmFactory(true));

  /** Virtual constructor */
  virtual LeastSquaresStrategy * clone() const;

  /** String converter */
  String __repr__() const;

  /** Compute the components alpha_k_p_ by projecting the model on the partial L2 basis */
  void computeCoefficients(const NumericalMathFunction & function,
                           const Basis & basis,
                           const Indices & indices,
                           const Indices & addedRanks,
                           const Indices & conservedRanks,
                           const Indices & removedRanks,
                           const UnsignedInteger marginalIndex = 0);

  /** Method save() stores the object through the StorageManager */
  void save(Advocate & adv) const;

  /** Method load() reloads the object from the StorageManager */
  void load(Advocate & adv);


protected:

private:
  /** Factory to build an ApproximationAlgorithmImplementation */
  ApproximationAlgorithmImplementationFactoryImplementation p_approximationAlgorithmImplementationFactory_;

} ; /* class LeastSquaresStrategy */


END_NAMESPACE_OPENTURNS

#endif /* OPENTURNS_LEASTSQUARESSTRATEGY_HXX */
