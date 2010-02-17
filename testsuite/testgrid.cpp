#include <grid.h>
#include <datastream.h>
#include <iostream>

void testmatrix()
{

  std::cout << "Testing Grid Access\n";
  
  typedef schnek::Grid<double, 1> Grid1;
  typedef schnek::Grid<double, 2> Grid2;
  typedef schnek::Grid<double, 3> Grid3;
  typedef schnek::Grid<double, 4> Grid4;
  typedef schnek::Grid<double, 5> Grid5;
  
  typedef Grid1::IndexType Ind1;
  typedef Grid2::IndexType Ind2;
  typedef Grid3::IndexType Ind3;
  typedef Grid4::IndexType Ind4;
  typedef Grid5::IndexType Ind5;
  
  Ind1 low1(0),            high1(6);
  Ind2 low2(1,2),          high2(6,5);
  Ind3 low3(1,1,5),        high3(3,2,8);
  Ind4 low4(2,3,1,2),      high4(5,7,3,4);
  Ind5 low5(-3,-2,2,1,-5), high5(0,0,5,5,-2);
  
  Grid1 G1(low1,high1);
  Grid2 G2(low2,high2);
  Grid3 G3(low3,high3);
  Grid4 G4(low4,high4);
  Grid5 G5(low5,high5);
  
  int i,j,k,l,m;
  
  for (i=low1[0]; i<=high1[0]; ++i) G1(i) = i*i; 

  for (i=low2[0]; i<=high2[0]; ++i)
    for (j=low2[1]; j<=high2[1]; ++j) G2(i,j) = i+j;
    
  for (i=low3[0]; i<=high3[0]; ++i)
    for (j=low3[1]; j<=high3[1]; ++j)
      for (k=low3[2]; k<=high3[2]; ++k) G3(i,j,k) = i+j+k;

  for (i=low4[0]; i<=high4[0]; ++i)
    for (j=low4[1]; j<=high4[1]; ++j)
      for (k=low4[2]; k<=high4[2]; ++k)
        for (l=low4[3]; l<=high4[3]; ++l) G4(i,j,k,l) = i+j+k+l;
      
  for (i=low5[0]; i<=high5[0]; ++i)
    for (j=low5[1]; j<=high5[1]; ++j)
      for (k=low5[2]; k<=high5[2]; ++k)
        for (l=low5[3]; l<=high5[3]; ++l)
          for (m=low5[4]; m<=high5[4]; ++m) G5(i,j,k,l,m) = i+j+k+l+m;

  
  std::cout << "Grid 1:\n" << G1 << "\n============\n";  
  std::cout << "Grid 2:\n" << G2 << "\n============\n";
  std::cout << "Grid 3:\n" << G3 << "\n============\n";
  std::cout << "Grid 4:\n" << G4 << "\n============\n";
  std::cout << "Grid 5:\n" << G5 << "\n============\n";

  std::cout << "Testing Grid storage_iterator\n";
  
  for (
    Grid2::storage_iterator it = G2.begin();
    it != G2.end();
    ++it
  )
  {
    std::cout << *it << " ";
  }
  std::cout << "\n";
} 

int main()
{
  testmatrix();
}
