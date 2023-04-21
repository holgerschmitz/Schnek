/*
 * test_range_c_iteration.cpp
 *
 *  Created on: 15 Dec 2022
 *      Author: Holger Schmitz
 */

#include "../utility.hpp"
#include "range_test_fixture.hpp"

#include <grid/iteration/range-iteration.hpp>
#include <grid/grid.hpp>
#include <grid/range.hpp>

#include <boost/timer/progress_display.hpp>

#include <boost/test/unit_test.hpp>

using namespace schnek;

BOOST_AUTO_TEST_SUITE( range_iteration )

BOOST_AUTO_TEST_SUITE( c_order )

BOOST_FIXTURE_TEST_CASE( iterate_1d, RangeIterationTest )
{
    typedef Grid<int, 1, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;

    GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);
    for (int n=0; n<10; ++n)
    {
        random_extent<1>(lo, hi);
        Range<int, 1, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);

        int count = 0;
        RangeCIterationPolicy<1>::forEach(range, [&](const GridType::IndexType& pos){
            grid[pos] = count++;
        });

        for (int i=lo[0]; i<=hi[0]; ++i)
        {
            BOOST_CHECK_EQUAL(grid(i), i - lo[0]);
        }
        ++show_progress;
    }
}

BOOST_FIXTURE_TEST_CASE( iterate_2d, RangeIterationTest )
{
    typedef Grid<int, 2, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;

    GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<10; ++n)
    {
        random_extent<2>(lo, hi);
        Range<int, 2, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        int count = 0;
        RangeCIterationPolicy<2>::forEach(range, [&](const GridType::IndexType& pos){
            grid[pos] = count++;
        });

        for (int i=lo[0]; i<=hi[0]; ++i)
        {
            for (int j=lo[1]; j<=hi[1]; ++j)
            {
                BOOST_CHECK_EQUAL(grid(i, j), j - lo[1] + (i-lo[0])*dims[1]);
            }
        }
        ++show_progress;
    }
}

BOOST_FIXTURE_TEST_CASE( iterate_3d, RangeIterationTest )
{
    typedef Grid<int, 3, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;

    GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<10; ++n)
    {
        random_extent<3>(lo, hi);
        Range<int, 3, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        int count = 0;
        RangeCIterationPolicy<3>::forEach(range, [&](const GridType::IndexType& pos){
            grid[pos] = count++;
        });

        for (int i=lo[0]; i<=hi[0]; ++i)
        {
            for (int j=lo[1]; j<=hi[1]; ++j)
            {
                for (int k=lo[2]; k<=hi[2]; ++k)
                {
                    BOOST_CHECK_EQUAL(grid(i, j, k), k - lo[2] + (j - lo[1] + (i-lo[0])*dims[1])*dims[2]);
                }
            }
        }
        ++show_progress;
    }
}

BOOST_FIXTURE_TEST_CASE( iterate_4d, RangeIterationTest )
{
    typedef Grid<int, 4, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;

    GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<10; ++n)
    {
        random_extent<4>(lo, hi);
        Range<int, 4, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        int count = 0;
        RangeCIterationPolicy<4>::forEach(range, [&](const GridType::IndexType& pos){
            grid[pos] = count++;
        });

        for (int i=lo[0]; i<=hi[0]; ++i)
        {
            for (int j=lo[1]; j<=hi[1]; ++j)
            {
                for (int k=lo[2]; k<=hi[2]; ++k)
                {
                    for (int l=lo[3]; l<=hi[3]; ++l)
                    {
                        BOOST_CHECK_EQUAL(grid(i, j, k, l), 
                          l - lo[3] + (k - lo[2] + (j - lo[1] + (i-lo[0])*dims[1])*dims[2])*dims[3]
                        );
                    }
                }
            }
        }
        ++show_progress;
    }
}


BOOST_FIXTURE_TEST_CASE( iterate_5d, RangeIterationTest )
{
    typedef Grid<int, 5, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;

    GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<10; ++n)
    {
        random_extent<5>(lo, hi);
        Range<int, 5, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        int count = 0;
        RangeCIterationPolicy<5>::forEach(range, [&](const GridType::IndexType& pos){
            grid[pos] = count++;
        });

        for (int i=lo[0]; i<=hi[0]; ++i)
        {
            for (int j=lo[1]; j<=hi[1]; ++j)
            {
                for (int k=lo[2]; k<=hi[2]; ++k)
                {
                    for (int l=lo[3]; l<=hi[3]; ++l)
                    {
                        for (int m=lo[4]; m<=hi[4]; ++m)
                        {
                            BOOST_CHECK_EQUAL(grid(i, j, k, l, m), 
                              m - lo[4] + (
                                l - lo[3] + (k - lo[2] + (j - lo[1] + (i-lo[0])*dims[1])*dims[2])*dims[3]
                              )*dims[4]
                            );
                        }
                    }
                }
            }
        }
        ++show_progress;
    }
}


BOOST_FIXTURE_TEST_CASE( iterate_6d, RangeIterationTest )
{
    typedef Grid<int, 6, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;

    GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<10; ++n)
    {
        random_extent<6>(lo, hi);
        Range<int, 6, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        int count = 0;
        RangeCIterationPolicy<6>::forEach(range, [&](const GridType::IndexType& pos){
            grid[pos] = count++;
        });

        for (int i=lo[0]; i<=hi[0]; ++i)
        {
            for (int j=lo[1]; j<=hi[1]; ++j)
            {
                for (int k=lo[2]; k<=hi[2]; ++k)
                {
                    for (int l=lo[3]; l<=hi[3]; ++l)
                    {
                        for (int m=lo[4]; m<=hi[4]; ++m)
                        {
                            for (int n=lo[5]; n<=hi[5]; ++n)
                            {
                                BOOST_CHECK_EQUAL(grid(i, j, k, l, m, n), 
                                  n - lo[5] + (m - lo[4] + (
                                    l - lo[3] + (k - lo[2] + (j - lo[1] + (i-lo[0])*dims[1])*dims[2])*dims[3]
                                  )*dims[4])*dims[5]
                                );
                            }
                        }
                    }
                }
            }
        }
        ++show_progress;
    }
}

BOOST_FIXTURE_TEST_CASE( iterate_7d, RangeIterationTest )
{
    typedef Grid<int, 7, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;

    GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<10; ++n)
    {
        random_extent<7>(lo, hi);
        Range<int, 7, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        int count = 0;
        RangeCIterationPolicy<7>::forEach(range, [&](const GridType::IndexType& pos){
            grid[pos] = count++;
        });

        for (int i=lo[0]; i<=hi[0]; ++i)
        {
            for (int j=lo[1]; j<=hi[1]; ++j)
            {
                for (int k=lo[2]; k<=hi[2]; ++k)
                {
                    for (int l=lo[3]; l<=hi[3]; ++l)
                    {
                        for (int m=lo[4]; m<=hi[4]; ++m)
                        {
                            for (int n=lo[5]; n<=hi[5]; ++n)
                            {
                                for (int o=lo[6]; o<=hi[6]; ++o)
                                {
                                    BOOST_CHECK_EQUAL(grid(i, j, k, l, m, n, o), 
                                      o - lo[6] + (n - lo[5] + (m - lo[4] + (
                                        l - lo[3] + (k - lo[2] + (j - lo[1] + (i-lo[0])*dims[1])*dims[2])*dims[3]
                                      )*dims[4])*dims[5])*dims[6]
                                    );
                                }
                            }
                        }
                    }
                }
            }
        }
        ++show_progress;
    }
}


BOOST_FIXTURE_TEST_CASE( iterate_8d, RangeIterationTest )
{
    typedef Grid<int, 8, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;

    GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<10; ++n)
    {
        random_extent<8>(lo, hi);
        Range<int, 8, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        int count = 0;
        RangeCIterationPolicy<8>::forEach(range, [&](const GridType::IndexType& pos){
            grid[pos] = count++;
        });

        for (int i=lo[0]; i<=hi[0]; ++i)
        {
            for (int j=lo[1]; j<=hi[1]; ++j)
            {
                for (int k=lo[2]; k<=hi[2]; ++k)
                {
                    for (int l=lo[3]; l<=hi[3]; ++l)
                    {
                        for (int m=lo[4]; m<=hi[4]; ++m)
                        {
                            for (int n=lo[5]; n<=hi[5]; ++n)
                            {
                                for (int o=lo[6]; o<=hi[6]; ++o)
                                {
                                    for (int p=lo[7]; p<=hi[7]; ++p)
                                    {
                                        BOOST_CHECK_EQUAL(grid(i, j, k, l, m, n, o, p), 
                                          p - lo[7] + (o - lo[6] + (n - lo[5] + (m - lo[4] + (
                                            l - lo[3] + (k - lo[2] + (j - lo[1] + (i-lo[0])*dims[1])*dims[2])*dims[3]
                                          )*dims[4])*dims[5])*dims[6])*dims[7]
                                        );
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        ++show_progress;
    }
}


BOOST_FIXTURE_TEST_CASE( iterate_9d, RangeIterationTest )
{
    typedef Grid<int, 9, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;

    GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<10; ++n)
    {
        random_extent<9>(lo, hi);
        Range<int, 9, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        int count = 0;
        RangeCIterationPolicy<9>::forEach(range, [&](const GridType::IndexType& pos){
            grid[pos] = count++;
        });

        for (int i=lo[0]; i<=hi[0]; ++i)
        {
            for (int j=lo[1]; j<=hi[1]; ++j)
            {
                for (int k=lo[2]; k<=hi[2]; ++k)
                {
                    for (int l=lo[3]; l<=hi[3]; ++l)
                    {
                        for (int m=lo[4]; m<=hi[4]; ++m)
                        {
                            for (int n=lo[5]; n<=hi[5]; ++n)
                            {
                                for (int o=lo[6]; o<=hi[6]; ++o)
                                {
                                    for (int p=lo[7]; p<=hi[7]; ++p)
                                    {
                                        for (int q=lo[8]; q<=hi[8]; ++q)
                                        {
                                            BOOST_CHECK_EQUAL(grid(i, j, k, l, m, n, o, p, q), 
                                              q - lo[8] + (p - lo[7] + (o - lo[6] + (n - lo[5] + (m - lo[4] + (
                                                l - lo[3] + (k - lo[2] + (j - lo[1] + (i-lo[0])*dims[1])*dims[2])*dims[3]
                                              )*dims[4])*dims[5])*dims[6])*dims[7])*dims[8]
                                            );
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        ++show_progress;
    }
}

BOOST_FIXTURE_TEST_CASE( iterate_10d, RangeIterationTest )
{
    typedef Grid<int, 10, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;

    GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<10; ++n)
    {
        random_extent<10>(lo, hi);
        Range<int, 10, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        int count = 0;
        RangeCIterationPolicy<10>::forEach(range, [&](const GridType::IndexType& pos){
            grid[pos] = count++;
        });

        for (int i=lo[0]; i<=hi[0]; ++i)
        {
            for (int j=lo[1]; j<=hi[1]; ++j)
            {
                for (int k=lo[2]; k<=hi[2]; ++k)
                {
                    for (int l=lo[3]; l<=hi[3]; ++l)
                    {
                        for (int m=lo[4]; m<=hi[4]; ++m)
                        {
                            for (int n=lo[5]; n<=hi[5]; ++n)
                            {
                                for (int o=lo[6]; o<=hi[6]; ++o)
                                {
                                    for (int p=lo[7]; p<=hi[7]; ++p)
                                    {
                                        for (int q=lo[8]; q<=hi[8]; ++q)
                                        {
                                            for (int r=lo[9]; r<=hi[9]; ++r)
                                            {
                                                BOOST_CHECK_EQUAL(grid(i, j, k, l, m, n, o, p, q, r), 
                                                  r - lo[9] + (q - lo[8] + (p - lo[7] + (o - lo[6] + (n - lo[5] + (m - lo[4] + (
                                                    l - lo[3] + (k - lo[2] + (j - lo[1] + (i-lo[0])*dims[1])*dims[2])*dims[3]
                                                  )*dims[4])*dims[5])*dims[6])*dims[7])*dims[8])*dims[9]
                                                );
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        ++show_progress;
    }
}



BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()