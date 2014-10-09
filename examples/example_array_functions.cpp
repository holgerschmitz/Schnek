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
    Array<double, 3> a(4., 5., 6.);
    Array<double, 3> b(2., 3., 4.);

    Array<double, 3> c;

    c = 2.*(a + b)/3.;

    c -= a;
    c *= 1.5;


    for (int i = 0; i < 3; ++i)
      std::cout << c[i] << " ";

    std::cout << std::endl;
  }

  // Example 2
  {
    Array<int, 3> a;

    a.fill(3);
    a.at(1) = 5;
    std::cout << "Product: " << a.product() << std::endl;
    std::cout << "Sum: " << a.sum() << std::endl;
    std::cout << "Square: " << a.sqr() << std::endl;

    Array<int,2> b = a.project<2>();

    std::cout << "a = ";
    for (int i = 0; i < 3; ++i)
      std::cout << a[i] << " ";

    std::cout << std::endl;

    std::cout << "b = ";
    for (int i = 0; i < 2; ++i)
      std::cout << b[i] << " ";

    std::cout << std::endl;
  }
}
