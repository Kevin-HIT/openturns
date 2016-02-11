//                                               -*- C++ -*-
/**
 * @brief ANCOVA implements the sensivity analysis method for correlated data
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

#ifndef OPENTURNS_ANCOVA_HXX
#define OPENTURNS_ANCOVA_HXX

#include "openturns/OTprivate.hxx"
#include "openturns/FunctionalChaosResult.hxx"

BEGIN_NAMESPACE_OPENTURNS

/**
 * @class ANCOVA
 *
 */

class OT_API ANCOVA
{
public:

  /** Some typedefs to ease reading */
  typedef Collection<NumericalMathFunction>     NumericalMathFunctionCollection;

  /** Constructor with model */
  ANCOVA(const FunctionalChaosResult & functionalChaosResult, const NumericalSample & correlatedInput);

  /** Accessor to ANCOVA indices measuring uncorrelated effects */
  NumericalPoint getUncorrelatedIndices(const UnsignedInteger marginalIndex = 0) const;

  /** Accessor to ANCOVA indices: total parts of variance of the output due to each input */
  NumericalPoint getIndices(const UnsignedInteger marginalIndex = 0) const;

private:

  /** Compute the indices */
  void run() const;

  /** Functional chaos result */
  FunctionalChaosResult functionalChaosResult_;

  /** correlated Input */
  NumericalSample correlatedInput_;

  /** Flag to check if the indices are already computed*/
  mutable Bool alreadyComputedIndices_;

  /** ANCOVA indices measuring uncorrelated effects */
  mutable NumericalSample uncorrelatedIndices_;

  /**  ANCOVA indices: total parts of variance of the output due to each input */
  mutable NumericalSample indices_;

}; /* class ANCOVA */

END_NAMESPACE_OPENTURNS

#endif /* OPENTURNS_ANCOVA_HXX */