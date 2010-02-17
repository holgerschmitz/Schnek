#include <matrix.h>
#include <datastream.h>
#include <iostream>

#include <matrix.h>
#include <mindex.h>

void testmatrix()
{

  std::cout << "Testing Matrix Access\n";
  
  typedef schnek::Matrix<double, 1> Mat1;
  typedef schnek::Matrix<double, 2> Mat2;
  typedef schnek::Matrix<double, 3> Mat3;
  typedef schnek::Matrix<double, 4> Mat4;
  typedef schnek::Matrix<double, 5> Mat5;
  
  typedef Mat1::IndexType Ind1;
  typedef Mat2::IndexType Ind2;
  typedef Mat3::IndexType Ind3;
  typedef Mat4::IndexType Ind4;
  typedef Mat5::IndexType Ind5;
  
  Ind1 low1(0),            high1(6);
  Ind2 low2(1,2),          high2(6,5);
  Ind3 low3(1,1,5),        high3(3,2,8);
  Ind4 low4(2,3,1,2),      high4(5,7,3,4);
  Ind5 low5(-3,-2,2,1,-5), high5(0,0,5,5,-2);
  
  Mat1 M1(low1,high1);
  Mat2 M2(low2,high2);
  Mat3 M3(low3,high3);
  Mat4 M4(low4,high4);
  Mat5 M5(low5,high5);
  
  int i,j,k,l,m;
  
  for (i=low1[0]; i<=high1[0]; ++i) M1(i) = i*i; 

  for (i=low2[0]; i<=high2[0]; ++i)
    for (j=low2[1]; j<=high2[1]; ++j) M2(i,j) = i+j;
    
  for (i=low3[0]; i<=high3[0]; ++i)
    for (j=low3[1]; j<=high3[1]; ++j)
      for (k=low3[2]; k<=high3[2]; ++k) M3(i,j,k) = i+j+k;

  for (i=low4[0]; i<=high4[0]; ++i)
    for (j=low4[1]; j<=high4[1]; ++j)
      for (k=low4[2]; k<=high4[2]; ++k)
        for (l=low4[3]; l<=high4[3]; ++l) M4(i,j,k,l) = i+j+k+l;
      
  for (i=low5[0]; i<=high5[0]; ++i)
    for (j=low5[1]; j<=high5[1]; ++j)
      for (k=low5[2]; k<=high5[2]; ++k)
        for (l=low5[3]; l<=high5[3]; ++l)
          for (m=low5[4]; m<=high5[4]; ++m) M5(i,j,k,l,m) = i+j+k+l+m;

  
  std::cout << "Matrix 1:\n" << M1 << "\n============\n";  
  std::cout << "Matrix 2:\n" << M2 << "\n============\n";
  std::cout << "Matrix 3:\n" << M3 << "\n============\n";
  std::cout << "Matrix 4:\n" << M4 << "\n============\n";
  std::cout << "Matrix 5:\n" << M5 << "\n============\n";

  std::cout << "Testing Matrix storage_iterator\n";
  
  for (
    Mat2::storage_iterator it = M2.begin();
    it != M2.end();
    ++it
  )
  {
    std::cout << *it << " ";
  }
  std::cout << "\n";
  
  std::cout << "Testing Matrix MIndex expressions\n";
    
  Mat2 M2a(Ind2(0,0),Ind2(5,5)), M2b(Ind2(0,0),Ind2(5,5));

  for (i=0; i<=5; ++i)
    for (j=0; j<=5; ++j) M2a(i,j) = i+j*j;

/*  MIndex s,t;

  M2b(s,t);
  std::cout << "Matrix 2a:\n" << M2a << "\n============\n";
  std::cout << "Matrix 2b:\n" << M2b << "\n============\n";
  
  Mat1 M1b(Ind1(0),Ind1(5));
  
  t() = 2;
  M1b(s) = M2a(s,2);
  std::cout << "Matrix 1b:\n" << M1b << "\n============\n";
*/
  
} 

int main()
{
  testmatrix();
}
