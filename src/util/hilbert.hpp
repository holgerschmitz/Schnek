/*
 * hilbert.hpp
 *
 * Created on: 26 Sep 2017
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
 */

#ifndef SCHNEK_UTIL_HILBERT_HPP_
#define SCHNEK_UTIL_HILBERT_HPP_

#include "../grid/array.hpp"

namespace schnek {

/** @brief Transform in-place between index on a Hilbert curve and geometrical axes
 *
 * The index is first converted to a Hilbert Transpose.
 *
 * Example: b=5 bits for each of n=3 coordinates.
 * 15-bit Hilbert integer = A B C D E F G H I J K L M N O is stored as its Transpose
 * X[0] = A D G J M
 * X[1] = B E H K N
 * X[2] = C F I L O
 *        high    low
 *
 * Based on public domain code by John Skilling.
 * Skilling, J., 2004, April. Programming the Hilbert curve.
 *   In AIP Conference Proceedings (Vol. 707, No. 1, pp. 381-387)
 */
class HilbertCurve
{
  public:

  /** @brief Convert a Hilbert Transpose to axes coordinates
   *
   * The conversion is done in-place. The input @X is replaced
   *
   * @param X Input the Hilbert transpose and output the axes coordinates
   * @param b The bit depth, i.e. the level of the curve
   */
  template<int dim, template<int> class CheckingPolicy>
  static void transposeToAxes(Array<unsigned long, dim, CheckingPolicy> &X, int b)
  {
    unsigned long N = 2 << (b-1), P, Q, t;
    int i;

    // Gray decode by H ^ (H/2)
    t = X[dim-1] >> 1;
    for( i = dim-1; i > 0; i-- ) X[i] ^= X[i-1];
    X[0] ^= t;

    // Undo excess work
    for( Q = 2; Q != N; Q <<= 1 )
    {
      P = Q - 1;
      for( i = dim-1; i >= 0 ; i-- )
        if( X[i] & Q )
        {
          X[0] ^= P; // invert
        }
        else
        {
          t = (X[0]^X[i]) & P;
          X[0] ^= t;
          X[i] ^= t;
        }
    } // exchange
  }

  /** @brief Convert axes coordinates to a Hilbert Transpose
   *
   * The conversion is done in-place. The input @X is replaced
   *
   * @param X Input the axes coordinates and output the Hilbert transpose
   * @param b The bit depth, i.e. the level of the curve
   */
  template<int dim, template<int> class CheckingPolicy>
  static void axesToTranspose(Array<unsigned long, dim, CheckingPolicy> &X, int b)
  {
    unsigned long M = 1 << (b-1), P, Q, t;
    int i;

    // Inverse undo
    for( Q = M; Q > 1; Q >>= 1 )
    {
      P = Q - 1;
      for( i = 0; i < dim; i++ )
      if( X[i] & Q )
      {
        X[0] ^= P; // invert
      }
      else
      {
        t = (X[0]^X[i]) & P;
        X[0] ^= t;
        X[i] ^= t;
      }
    } // exchange

    // Gray encode
    for( i = 1; i < dim; i++ ) X[i] ^= X[i-1];

    t = 0;
    for( Q = M; Q > 1; Q >>= 1 )
      if( X[dim-1] & Q ) t ^= Q-1;

    for( i = 0; i < dim; i++ ) X[i] ^= t;
  }

  /** @brief Convert Hilbert curve index to a Hilbert Transpose
   *
   * The parameter @X is filled with the result.
   *
   * @param X Outputs the Hilbert transpose
   * @param index The Hilbert curve index
   * @param b The bit depth, i.e. the level of the curve
   */
  template<int dim, template<int> class CheckingPolicy>
  static void indexToTranspose(Array<unsigned long, dim, CheckingPolicy> &X, unsigned long index, int b)
  {
    X = 0;
    unsigned long v = index;
    for (int bi=0; bi<b; ++bi)
    {
      for (int d=dim-1; d>=0; --d)
      {
        X[d] |= (v & 1) << bi;
        v = v>>1;
      }
    }
  }

  /** @brief Convert Hilbert Transpose to a Hilbert curve index
   *
   * @param X The Hilbert transpose
   * @param b The bit depth, i.e. the level of the curve
   * @return The Hilbert curve index
   */
  template<int dim, template<int> class CheckingPolicy>
  static unsigned long transposeToIndex(const Array<unsigned long, dim, CheckingPolicy> &X, int b)
  {
    unsigned long v = 0;
    for (int bi=b-1; bi>=0; --bi)
    {
      for (int d=0; d<dim; ++d)
      {
        v = v<<1;
        v |= (X[d] & (1<<bi)) >> bi;
      }
    }
    return v;
  }

  /** @brief Convert Hilbert curve index to axes coordinates
   *
   * The parameter @X is filled with the result.
   *
   * @param X Outputs the axes coordinates
   * @param index The Hilbert curve index
   * @param b The bit depth, i.e. the level of the curve
   */
  template<int dim, template<int> class CheckingPolicy>
  static void indexToAxes(Array<unsigned long, dim, CheckingPolicy> &X, unsigned long index, int b)
  {
    indexToTranspose(X, index, b);
    transposeToAxes(X, b);
  }

  /** @brief Convert Hilbert axes coordinates to curve index
   *
   * The parameter @X, passed by reference, is destroyed.
   *
   * @param X Input the axes coordinates
   * @param b The bit depth, i.e. the level of the curve
   * @return The Hilbert curve index
   */
  template<int dim, template<int> class CheckingPolicy>
  static unsigned long axesToIndex(Array<unsigned long, dim, CheckingPolicy> &X, int b)
  {
    axesToTranspose(X, b);
    return transposeToIndex(X, b);
  }
};


} // namespace schnek

#endif /* SCHNEK_UTIL_HILBERT_HPP_ */
