/*
 * factor.hpp
 *
 * Created on: 3 Oct 2012
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2012 Holger Schmitz
 *
 * This file is part of Schnek.
 *
 * Schnek is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Schnek is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Schnek.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SCHNEK_FACTOR_HPP_
#define SCHNEK_FACTOR_HPP_

#include <vector>

namespace schnek {

/**
 * Create a balanced vector of integers that factorises a given number as evenly as possible
 *
 * @param number  the number to be factorised
 * @param nfact   the number of factors to divide into
 * @param factors the output vector containing the factors
 * @param weights a vector of weights for each factor. If a weight for a given factor is \f$w\f$, then
 *                the algorithm attempts to create a factor \f$w\f$ times larger than the factors of
 *                weight 1
 */
void equalFactors
  (
    int number,
    int nfact,
    std::vector<int> &factors,
    std::vector<int> &weights
  );

} // namespace schnek

#endif // SCHNEK_FACTOR_HPP_
