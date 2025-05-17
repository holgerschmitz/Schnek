/*
 * example_grids_basics.cpp
 *
 *  Created on: 2 Jul 2014
 *      Author: Holger Schmitz
 *       Email: holger@notjustphysics.com
 */

 #include "../src/grid/range.hpp"
 #include "../src/grid.hpp"
 #include "../src/grid/subgrid.hpp"

 #include <iostream>
 #include <cmath>
 #include <chrono>
 
 using namespace schnek;
 using namespace std::chrono;
 
 int main()
 {  int n  = 100;

    typedef schnek::Grid<double, 3> MyGrid;
    typedef MyGrid::IndexType MyIndex;
 
    MyGrid grid1(MyIndex(n, n, n));
    MyGrid grid2(MyIndex(n, n, n));
     
    grid1 = 100;
    grid2 = 100;

    Range<int, 3>::LimitType low(0, 0, 0);
    Range<int, 3>::LimitType high(n-1, n-1, n-1);
    Range<int, 3> range(low, high);

    std::cout<<"Test"<<std::endl;

    // Case 1
    auto beg = high_resolution_clock::now();

    for (auto &pos : range){
        grid1[pos] = exp(grid1[pos]) * sin(2 * (pos[0] + pos[1] + pos[2]) * M_PI);
    }

    auto end = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>(end - beg);

    std::cout << "Case 1: Elapsed Time: " << duration.count() << "  |  " << grid1(100, 100, 100) << std::endl;

    grid1 = 100;

    // Case 2
    beg = high_resolution_clock::now();

    for (Range<int, 3>::iterator it=range.begin(); it!=range.end(); ++it)
    {
        const Range<int, 3>::LimitType &pos = *it;

        // grid1(pos[0], pos[1], pos[2]) = exp(grid1(pos[0], pos[1], pos[2])) * sin(2 * M_PI);
        grid1[pos] = exp(grid1[pos]) * sin(2 * (pos[0] + pos[1] + pos[2]) * M_PI);

        // std::cout << grid1(pos[0], pos[1], pos[2]) << std::endl;
    }

    // std::cout << grid1(10, 10, 10) << std::endl;

    end = high_resolution_clock::now();

    duration = duration_cast<milliseconds>(end - beg);

    std::cout << "Case 2: Elapsed Time: " << duration.count() << "  |  " << grid1(100, 100, 100) << std::endl;

    // Case 3
    beg = high_resolution_clock::now();

    for (int i=grid2.getLo(0); i<=grid2.getHi(0); ++i)
        for (int j=grid2.getLo(1); j<=grid2.getHi(1); ++j)
            for (int k=grid2.getLo(2); k<=grid2.getHi(2); ++k){
                grid2(i, j, k) = exp(grid2(i, j, k)) * sin(2 * (i + j + k) * M_PI);

                // std::cout << grid2(i, j, k) << std::endl;
            }
    
    // std::cout << grid2(10, 10, 10) << std::endl;

    end = high_resolution_clock::now();

    duration = duration_cast<milliseconds>(end - beg);

    std::cout << "Case 3: Elapsed Time: " << duration.count() << "  |  " << grid2(100, 100, 100) << std::endl;

    // for (Range<int, 2>::iterator it=range.begin(); it!=range.end(); ++it)
    // {
    //     const Range<int, 2>::LimitType &pos = *it;

    //     grid2(pos[0], pos[1]) += grid1(pos[0], pos[1]);

    //     std::cout << grid2(pos[0], pos[1]) << std::endl;
    // }
 }
 