/*
 * utility.hpp
 *
 *  Created on: 2 Aug 2017
 *      Author: Holger Schmitz
 */

#include "utility.hpp"

#include <grid/iteration/kokkos-iteration.hpp>

#include <cmath>
bool is_equal(double a, double b)
{
  return ((a==0.0) && (b==0.0)) ||
      ( fabs(a-b)/(fabs(a)+fabs(b)) < 100*std::numeric_limits<double>::epsilon() );
}
