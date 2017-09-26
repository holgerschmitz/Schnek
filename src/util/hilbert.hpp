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

namespace schnek {

template<int dim>
void TransposeToAxes(Array<unsigned long, dim> &X, int b)
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

template<int dim>
void AxesToTranspose(Array<unsigned long, dim> &X, int b)
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

template<int dim>
void IndexToTranspose(Array<unsigned long, dim> &X, unsigned long index, int b)
{
  X = 0;
  unsigned long v = index;
  for (int bi=0; bi<b; ++bi)
  {
    for (int d=dim-1; d>=0; --d)
    {
      X[d] |= (v & 1) << b;
      v = v>>1;
    }
  }
}

template<int dim>
unsigned long TransposeToIndex(const Array<unsigned long, dim> &X, int b)
{

}


} // namespace schnek

#endif /* SCHNEK_UTIL_HILBERT_HPP_ */
