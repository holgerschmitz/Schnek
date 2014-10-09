/*
 * example_array.cpp
 *
 *  Created on: 14 Jul 2014
 *      Author: Holger Schmitz
 *       Email: holger@notjustphysics.com
 */

#include "../src/grid/grid.hpp"

using namespace schnek;

int main()
{
  // Example 1
  {
    Grid<double, 3> grid(Grid<double, 3>::IndexType(0, 0, 0),
        Grid<double, 3>::IndexType(10, 10, 10));

    grid = 0.0;

    Grid<double, 3>::IndexType index(5, 5, 5);

    for (int i = 0; i <= 10; i++)
    {
      index[1] = i;
      grid[index] = 1.0;
    }
  }

  // Example 2
  {
    Array<int, 3> a;

    for (int i = 0; i < 3; ++i)
      a[i] = 2 * i + 1;

    Array<int, 3> b(2, 3, 4);
    Array<std::string, 5> s("a","b","c","d","e");
  }

  // Example 3
  {
    Array<int, 3> a(4, 5, 6);
    Array<int, 3> b(2, 3, 4);

    Array<int, 3> c;

    c = a + b;
    for (int i = 0; i < 3; ++i)
      std::cout << c[i] << " ";

    std::cout << std::endl;
  }

}
