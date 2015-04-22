#include <grid/grid.hpp>
#include <iostream>


#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>

#include <limits>

boost::random::mt19937 rGen;
boost::random::uniform_real_distribution<> dist(-1.0,1.0);

// safe comparison of two floating point numbers
bool compare_double(double a, double b)
{
  return ((a==0.0) && (b==0.0)) ||
      ( fabs(a-b)/(fabs(a)+fabs(b)) < 100*std::numeric_limits<double>::epsilon() );
}

void test_access_1d(schnek::Grid<double, 1> &grid)
{
  double sum_direct = 0.0;

  schnek::Grid<double, 1>::IndexType lo = grid.getLo();
  schnek::Grid<double, 1>::IndexType hi = grid.getHi();

  // write random numbers
  for (int i=lo[0]; i<=hi[0]; ++i)
  {
    double val = dist(rGen);
    grid(i) = val;
    sum_direct += val;
  }

  double sum_grid = 0.0;

  // read back random numbers
  for (int i=lo[0]; i<=hi[0]; ++i)
  {
    sum_grid += grid(i);
  }

  if (!compare_double(sum_direct, sum_grid))
  {
    std::cerr << "Sum over grid does not match direct sum (dim=1)\n";
    exit(-1);
  }
}


void test_access_2d(schnek::Grid<double, 2> &grid)
{
  double sum_direct = 0.0;

  schnek::Grid<double, 2>::IndexType lo = grid.getLo();
  schnek::Grid<double, 2>::IndexType hi = grid.getHi();

  // write random numbers
  for (int i=lo[0]; i<=hi[0]; ++i)
    for (int j=lo[1]; j<=hi[1]; ++j)
    {
      double val = dist(rGen);
      grid(i,j) = val;
      sum_direct += val;
    }

  double sum_grid = 0.0;

  // read back random numbers
  for (int i=lo[0]; i<=hi[0]; ++i)
    for (int j=lo[1]; j<=hi[1]; ++j)
    {
      sum_grid += grid(i,j);
    }

  if (!compare_double(sum_direct, sum_grid))
  {
    std::cerr << "Sum over grid does not match direct sum (dim=2)\n";
    exit(-1);
  }
}


void test_access_3d(schnek::Grid<double, 3> &grid)
{
  double sum_direct = 0.0;

  schnek::Grid<double, 3>::IndexType lo = grid.getLo();
  schnek::Grid<double, 3>::IndexType hi = grid.getHi();

  // write random numbers
  for (int i=lo[0]; i<=hi[0]; ++i)
    for (int j=lo[1]; j<=hi[1]; ++j)
      for (int k=lo[2]; k<=hi[2]; ++k)
      {
        double val = dist(rGen);
        grid(i,j,k) = val;
        sum_direct += val;
      }

  double sum_grid = 0.0;

  // read back random numbers
  for (int i=lo[0]; i<=hi[0]; ++i)
    for (int j=lo[1]; j<=hi[1]; ++j)
      for (int k=lo[2]; k<=hi[2]; ++k)
      {
        sum_grid += grid(i,j,k);
      }

  if (!compare_double(sum_direct, sum_grid))
  {
    std::cerr << "Sum over grid does not match direct sum (dim=3)\n";
    exit(-1);
  }
}


void test_access_4d(schnek::Grid<double, 4> &grid)
{
  double sum_direct = 0.0;

  schnek::Grid<double, 4>::IndexType lo = grid.getLo();
  schnek::Grid<double, 4>::IndexType hi = grid.getHi();

  // write random numbers
  for (int i=lo[0]; i<=hi[0]; ++i)
    for (int j=lo[1]; j<=hi[1]; ++j)
      for (int k=lo[2]; k<=hi[2]; ++k)
        for (int l=lo[3]; l<=hi[3]; ++l)
        {
          double val = dist(rGen);
          grid(i,j,k,l) = val;
          sum_direct += val;
        }

  double sum_grid = 0.0;

  // read back random numbers
  for (int i=lo[0]; i<=hi[0]; ++i)
    for (int j=lo[1]; j<=hi[1]; ++j)
      for (int k=lo[2]; k<=hi[2]; ++k)
      for (int l=lo[3]; l<=hi[3]; ++l)
        {
          sum_grid += grid(i,j,k,l);
        }

  if (!compare_double(sum_direct, sum_grid))
  {
    std::cerr << "Sum over grid does not match direct sum (dim=4)\n";
    exit(-1);
  }
}


void test_access_5d(schnek::Grid<double, 5> &grid)
{
  double sum_direct = 0.0;

  schnek::Grid<double, 5>::IndexType lo = grid.getLo();
  schnek::Grid<double, 5>::IndexType hi = grid.getHi();

  // write random numbers
  for (int i=lo[0]; i<=hi[0]; ++i)
    for (int j=lo[1]; j<=hi[1]; ++j)
      for (int k=lo[2]; k<=hi[2]; ++k)
        for (int l=lo[3]; l<=hi[3]; ++l)
          for (int m=lo[4]; m<=hi[4]; ++m)
          {
            double val = dist(rGen);
            grid(i,j,k,l,m) = val;
            sum_direct += val;
          }

  double sum_grid = 0.0;

  // read back random numbers
  for (int i=lo[0]; i<=hi[0]; ++i)
    for (int j=lo[1]; j<=hi[1]; ++j)
      for (int k=lo[2]; k<=hi[2]; ++k)
        for (int l=lo[3]; l<=hi[3]; ++l)
          for (int m=lo[4]; m<=hi[4]; ++m)
          {
            sum_grid += grid(i,j,k,l,m);
          }

  if (!compare_double(sum_direct, sum_grid))
  {
    std::cerr << "Sum over grid does not match direct sum (dim=5)\n";
    exit(-1);
  }
}

template<int rank>
void random_extent(typename schnek::Grid<double, rank>::IndexType &lo, typename schnek::Grid<double, rank>::IndexType &hi)
{
  const int maxExtent = (int)pow(1000000,(1.0/(double)rank));
  boost::random::uniform_int_distribution<> orig(-maxExtent/2, maxExtent/2);
  boost::random::uniform_int_distribution<> extent(1, maxExtent);
  for (int i=0; i<rank; ++i)
  {
    int o = orig(rGen);
    int l = extent(rGen);
    lo[i] = o;
    hi[i] = o+l;
  }
}

void testmatrix()
{
  schnek::Grid<double, 1>::IndexType lo1, hi1;
  schnek::Grid<double, 2>::IndexType lo2, hi2;
  schnek::Grid<double, 3>::IndexType lo3, hi3;
  schnek::Grid<double, 4>::IndexType lo4, hi4;
  schnek::Grid<double, 5>::IndexType lo5, hi5;

  for (int n=0; n<10; ++n)
  {
    random_extent<1>(lo1, hi1);
    random_extent<2>(lo2, hi2);
    random_extent<3>(lo3, hi3);
    random_extent<4>(lo4, hi4);
    random_extent<5>(lo5, hi5);
    schnek::Grid<double, 1> g1(lo1,hi1);
    schnek::Grid<double, 2> g2(lo2,hi2);
    schnek::Grid<double, 3> g3(lo3,hi3);
    schnek::Grid<double, 4> g4(lo4,hi4);
    schnek::Grid<double, 5> g5(lo5,hi5);
    test_access_1d(g1);
    test_access_2d(g2);
    test_access_3d(g3);
    test_access_4d(g4);
    test_access_5d(g5);

    for (int m=0; m<10; ++m)
    {
      random_extent<1>(lo1, hi1);
      random_extent<2>(lo2, hi2);
      random_extent<3>(lo3, hi3);
      random_extent<4>(lo4, hi4);
      random_extent<5>(lo5, hi5);

      g1.resize(lo1,hi1);
      g2.resize(lo2,hi2);
      g3.resize(lo3,hi3);
      g4.resize(lo4,hi4);
      g5.resize(lo5,hi5);

      test_access_1d(g1);
      test_access_2d(g2);
      test_access_3d(g3);
      test_access_4d(g4);
      test_access_5d(g5);
    }
  }
  
} 

int main()
{
  testmatrix();
  return 0; //success
}
