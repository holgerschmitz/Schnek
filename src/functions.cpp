#include "functions.h"

#include <cmath>
#include <cstdlib>


static const double RAND_MAX2 = double(RAND_MAX)*double(RAND_MAX);

double schnek::drand()
{
  return (double(rand()) +  double(RAND_MAX)*double(rand()))/RAND_MAX2;
}

int schnek::irand(int range)
{
  return int(drand()*range);
}
