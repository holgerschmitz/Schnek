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
    Array<double, 3> a;
    Array<double, 3> b(1.0, 2.0, 3.0);
    Array<std::string, 5> s("a","b","c","d","e");

    for (int i = 0; i < 3; ++i)
      a[i] = 2 * i + 1;


    Array<double, 3> c;

    c = 2.0*a + b;
    for (int i = 0; i < 3; ++i)
      std::cout << c[i] << " ";

    std::cout << std::endl;

}
