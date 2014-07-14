/*
 * example_grids_basics.cpp
 *
 *  Created on: 2 Jul 2014
 *      Author: Holger Schmitz
 *       Email: holger@notjustphysics.com
 */

#include "../src/grid.hpp"

int main()
{
  // Example 1
  {
    schnek::Grid<double, 2> grid(schnek::Grid<double, 2>::IndexType(5, 5));

    grid = 1;

    for (int j = 0; j < 5; ++j)
      grid(j, j) = sqrt(2 * j);

    for (int i = 0; i < 5; ++i)
    {
      for (int j = 0; j < 5; ++j)
        std::cout << grid(i, j) << " ";
      std::cout << std::endl;
    }
  }

  // Example 2
  {
    typedef schnek::Grid<double, 2> MyGrid;
    typedef MyGrid::IndexType MyIndex;

    MyGrid grid(MyIndex(5, 5));

    grid = 1;

    for (int j = 0; j < 5; ++j)
      grid(j, j) = sqrt(2 * j);

    for (int i = 0; i < 5; ++i)
    {
      for (int j = 0; j < 5; ++j)
        std::cout << grid(i, j) << " ";
      std::cout << std::endl;
    }
  }
}
