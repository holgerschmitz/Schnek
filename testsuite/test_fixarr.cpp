

#include <grid/array.hpp>
#include <grid/arrayexpression.hpp>
#include <iostream>
#include <vector>
#include <cmath>

using namespace schnek;

template<int size>
    struct SizeDummy
{};


void testConstructor()
{
  Array<int, 3>  A(2,6,24);
  Array<int, 5>  B(4,9,16,25,36);
  Array<int, 10> C(100,81,64,49,36,25,16,9,4,1);
  
  int i;
  int x,y;
  
  x=1;
  y=1;
  for (i=0;i<3;++i)
  {
    ++y;
    x = x*y;
    if (A[i] != x)
    {
      std::cerr << "FixedArray Constructor(3) Corrupt!";
      std::cerr << "Index "<<i<<" is "<<A[i]<<" but should be "<<x<<std::endl;
      exit(-1);
    }
  }

  y=1;
  for (i=0;i<5;++i)
  {
    ++y;
    x = y*y;
    if (B[i] != x)
    {
      std::cerr << "FixedArray Constructor(5) Corrupt!";
      std::cerr << "Index "<<i<<" is "<<B[i]<<" but should be "<<x<<std::endl;
      exit(-1);
    }
  }

  y=10;
  for (i=0;i<10;++i)
  {
    x = y*y;
    --y;
    if (C[i] != x)
    {
      std::cerr << "FixedArray Constructor(10) Corrupt!";
      std::cerr << "Index "<<i<<" is "<<C[i]<<" but should be "<<x<<std::endl;
      exit(-1);
    }
  }

}

template<int size>
    void testExpression(const SizeDummy<size> &, bool verbose=false)
{
  std::vector<int> result(size);
  int i;
  
  Array<int, size> A;
  Array<int, size> B;
  Array<int, size> C;
  Array<int, size> D;
  for (i=0; i<size; ++i)
  {
    int a = rand() % 10000;
    int b = rand() % 10000;
    A[i] = a;
    B[i] = b;
    result[i] = a + b;
  }
  
  D = A + B;

  for (i=0; i<size; ++i)
  {
    if (verbose) std::cout << A[i] << " + " << B[i] << " = " << D[i] << std::endl;
    if (D[i] != result[i])
    {
      std::cerr << "FixedArray Expression Addition Corrupt!";
      exit(-1);
    }
  }
  
  for (i=0; i<size; ++i)
  {
    int a = rand() % 10000;
    int b = rand() % 10000;
    A[i] = a;
    B[i] = b;
    result[i] = a - b;
  }
  
  D = A - B;
  for (i=0; i<size; ++i)
  {
    if (verbose) std::cout << A[i] << " - " << B[i] << " = " << D[i] << std::endl;
    if (D[i] != result[i])
    {
      std::cerr << "FixedArray Expression Subtraction Corrupt!";
      exit(-1);
    }
  }


  for (i=0; i<size; ++i)
  {
    int a = rand() % 10000;
    int b = rand() % 10000;
    int c = rand() % 10000;
    A[i] = a;
    B[i] = b;
    C[i] = c;
    result[i] = a + b - c;
  }
  
  D = A + B - C;
  for (i=0; i<size; ++i)
  {
    if (verbose) std::cout << A[i] << " + " << B[i] << " - " << C[i] << " = " << D[i] << std::endl;
    if (D[i] != result[i])
    {
      std::cerr << "FixedArray Expression Corrupt!";
      exit(-1);
    }
  }
}

void testfixedarr()
{
  int cnt;
  std::cout << "===================================" << std::endl;
  std::cout << "        Testing FixedArray" << std::endl;
  std::cout << "===================================" << std::endl;

  std::cout << "Testing Constructor" << std::endl;
  
  testConstructor();
  
  std::cout << "Testing FixedArray Expression" << std::endl;

  std::cout << "3"; 
  for (cnt = 0; cnt<100; ++cnt) testExpression(SizeDummy<3>());

  std::cout << " 5"; 
  for (cnt = 0; cnt<100; ++cnt) testExpression(SizeDummy<5>());

  std::cout << " 10"; 
  for (cnt = 0; cnt<100; ++cnt) testExpression(SizeDummy<10>());

  std::cout << " 50"; 
  for (cnt = 0; cnt<100; ++cnt) testExpression(SizeDummy<50>());

  std::cout << " 500" << std::endl; 
  for (cnt = 0; cnt<100; ++cnt) testExpression(SizeDummy<500>());

  std::cout << "  -- example --" << std::endl;
  testExpression(SizeDummy<10>(), true);
  
  exit(0);
}

int main()
{
  testfixedarr();
}
