/*
 * interpolate1d.hpp
 *
 *  Created on: 22 Jan 2020
 *  Author: Holger Schmitz
 */

#ifndef SCHNEK_UTIL_INTERPOLATE_INTERPOLATE1D_HPP_
#define SCHNEK_UTIL_INTERPOLATE_INTERPOLATE1D_HPP_

/**
 * Returns the index after which a value can be inserted into an array.
 *
 * If the value is smaller than the smallest value in the array, the returned
 * index will fall outside the allowed index range.
 */
template<class Grid1d>
inline int findInsertIndex(const Grid1d &X, double x)
{
  int lo = X.getLo(0);
  int hi = X.getHi(0);

  while (lo <= hi) {
    int mid = (hi + lo) / 2;

    if (x < X(mid))
    {
      hi = mid - 1;
    }
    else
    {
      lo = mid + 1;
    }
  }
  return hi;
}

template<class Grid1d>
inline double linearInterpolate(const Grid1d &X, const Grid1d &Y, double x)
{
  if (x<=X(X.getLo(0))) return Y(X.getLo(0));
  if (x>=X(X.getHi(0))) return Y(X.getHi(0));
  int p = findInsertIndex(X, x);
  return (Y(p+1) - Y(p))*(x - X(p))/(X(p+1) - X(p)) + Y(p);
}

#endif /* SCHNEK_UTIL_INTERPOLATE_INTERPOLATE1D_HPP_ */
