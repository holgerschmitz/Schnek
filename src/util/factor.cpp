/*
 * factor.cpp
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

#include "factor.hpp"
#include "algohelper.hpp"

#include <list>
#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace schnek;

void makePrimes(std::list<int> &primes, int max);

void factorize(int number, std::list<int> &primes, std::list<int> &factors);

double distribute
  (
    std::vector<int> &factors,
    std::list<int> allfact,
    std::vector<int> &weights
  );

void schnek::equalFactors
  (
    int number,
    int nfact,
    std::vector<int> &factors,
    std::vector<int> &weights
  )
{
  std::list<int> primes;
  std::list<int> allfact;
  makePrimes(primes, int(floor(sqrt(number))));
  factorize(number, primes, allfact);

  factors.resize(nfact);

  // this happens only on single processor runs
  if (allfact.empty())
  {
    for (int i=0; i<nfact; ++i) factors[i] = 1;
    return;
  }

  factors[0] = allfact.front();
  for (int i=1; i<nfact; ++i) factors[i] = 1;

  allfact.pop_front();
  if (allfact.empty()) return;
  distribute(factors, allfact, weights);
}

void makePrimes(std::list<int> &primes, int max)
{
  std::vector<bool> isprime(max+1,true);
  primes.clear();
  for (int i=2; i<=max; ++i)
  {
    if (isprime[i])
    {
      primes.push_back(i);
      int prod = 2*i;
      for (prod = 2*i; prod<=max; prod += i)
      {
        isprime[prod] = false;
      }
    }
  }
}

void factorize(int number, std::list<int> &primes, std::list<int> &factors)
{
  if (number == 1) return;
  std::list<int>::iterator it = primes.begin();

  while (it != primes.end())
  {
    int p = *it;
    if ((number % p) == 0)
    {
      factorize(number/p, primes, factors);
      factors.push_front(p);
      return;
    }
    ++it;
  }
  factors.push_front(number);
}

double distribute
  (
    std::vector<int> &factors,
    std::list<int> allfact,
    std::vector<int> &weights
  )
{
  int f = allfact.front();
  allfact.pop_front();

  if (allfact.empty())
  {
    std::vector<double> dfactors;
    dfactors.resize(factors.size());

    int imin = 0;
    double facmin = 0;

    for (size_t i=0; i<factors.size(); ++i)
    {
      dfactors[i] = double(factors[i])/double(weights[i]);
      if ((i==0) || (dfactors[i] < facmin))
      {
        imin = i;
        facmin = dfactors[i];
      }
    }

    dfactors[imin] *= f;
    factors[imin] *= f;

    return std::for_each(dfactors.begin(), dfactors.end(), calcsum<double>()).result();

  }
  else
  {
    std::vector<int> best_factors = factors;
    best_factors[0] *= f;
    double bestSum = distribute(best_factors, allfact, weights);

    for (size_t i=1; i<factors.size(); ++i)
    {
      std::vector<int> t_factors = factors;
      t_factors[i] *= f;
      double sum = distribute(t_factors, allfact, weights);
      if (sum < bestSum)
      {
        best_factors = t_factors;
        bestSum = sum;
      }
    }

    factors = best_factors;
    return bestSum;
  }
}

/*
// Testing program for factors
int main()
{
  std::vector<int> factors;
  int number = 64;
  int nfact = 3;

  std::vector<int> weights(nfact);
  weights[0] = 32;
  weights[1] = 32;
  weights[2] = 256;

  equalFactors(number, 3, factors, weights);

  std::cout << number << " = ";
  for (int i = 0; i < nfact; ++i)
  {
    std::cout << factors[i] << "*";
  }
    std::cout << std::endl;
}
*/




