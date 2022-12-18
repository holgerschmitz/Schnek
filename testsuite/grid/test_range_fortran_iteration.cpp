/*
 * test_range_fortran_iteration.cpp
 *
 *  Created on: 16 Dec 2022
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

BOOST_AUTO_TEST_SUITE( fortran_order )

BOOST_FIXTURE_TEST_CASE( iterate_1d, RangeIterationTest )
{
    typedef Grid<int, 1, GridBoostTestCheck, schnek::SingleArrayGridStorage> GridType;

    GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);
    for (int n=0; n<30; ++n)
    {
        random_extent<1>(lo, hi);
        Range<int, 1, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);

        int count = 0;
        RangeFortranIterationPolicy<1>::forEach(range, [&](const GridType::IndexType& pos){
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

    for (int n=0; n<30; ++n)
    {
        random_extent<2>(lo, hi);
        Range<int, 2, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        int count = 0;
        RangeFortranIterationPolicy<2>::forEach(range, [&](const GridType::IndexType& pos){
            grid[pos] = count++;
        });

        for (int i=lo[0]; i<=hi[0]; ++i)
        {
            for (int j=lo[1]; j<=hi[1]; ++j)
            {
                BOOST_CHECK_EQUAL(grid(i, j), i - lo[0] + (j-lo[1])*dims[0]);
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

    for (int n=0; n<30; ++n)
    {
        random_extent<3>(lo, hi);
        Range<int, 3, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        int count = 0;
        RangeFortranIterationPolicy<3>::forEach(range, [&](const GridType::IndexType& pos){
            grid[pos] = count++;
        });

        for (int i=lo[0]; i<=hi[0]; ++i)
        {
            for (int j=lo[1]; j<=hi[1]; ++j)
            {
                for (int k=lo[2]; k<=hi[2]; ++k)
                {
                    BOOST_CHECK_EQUAL(grid(i, j, k), i - lo[0] + (j-lo[1] + (k - lo[2])*dims[1])*dims[0]);
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

    for (int n=0; n<30; ++n)
    {
        random_extent<4>(lo, hi);
        Range<int, 4, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        int count = 0;
        RangeFortranIterationPolicy<4>::forEach(range, [&](const GridType::IndexType& pos){
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
                            i - lo[0] + (j-lo[1] + (k - lo[2] + (l - lo[3])*dims[2])*dims[1])*dims[0]
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

    for (int n=0; n<30; ++n)
    {
        random_extent<5>(lo, hi);
        Range<int, 5, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        int count = 0;
        RangeFortranIterationPolicy<5>::forEach(range, [&](const GridType::IndexType& pos){
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
                                i - lo[0] + (j-lo[1] + (k - lo[2] + (l - lo[3]
                                    + (m - lo[4])*dims[3]
                                )*dims[2])*dims[1])*dims[0]
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

    for (int n=0; n<30; ++n)
    {
        random_extent<6>(lo, hi);
        Range<int, 6, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        int count = 0;
        RangeFortranIterationPolicy<6>::forEach(range, [&](const GridType::IndexType& pos){
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
                                    i - lo[0] + (j-lo[1] + (k - lo[2] + (l - lo[3]
                                        + (m - lo[4] + (n - lo[5])*dims[4])*dims[3]
                                    )*dims[2])*dims[1])*dims[0]
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

    for (int n=0; n<30; ++n)
    {
        random_extent<7>(lo, hi);
        Range<int, 7, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        int count = 0;
        RangeFortranIterationPolicy<7>::forEach(range, [&](const GridType::IndexType& pos){
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
                                        i - lo[0] + (j-lo[1] + (k - lo[2] + (l - lo[3]
                                            + (m - lo[4] + (n - lo[5] + (o - lo[6])*dims[5])*dims[4])*dims[3]
                                        )*dims[2])*dims[1])*dims[0]
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

    for (int n=0; n<30; ++n)
    {
        random_extent<8>(lo, hi);
        Range<int, 8, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        int count = 0;
        RangeFortranIterationPolicy<8>::forEach(range, [&](const GridType::IndexType& pos){
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
                                            i - lo[0] + (j-lo[1] + (k - lo[2] + (l - lo[3]
                                                + (m - lo[4] + (n - lo[5] + (o - lo[6] + (p - lo[7])*dims[6])*dims[5])*dims[4])*dims[3]
                                            )*dims[2])*dims[1])*dims[0]
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

    for (int n=0; n<30; ++n)
    {
        random_extent<9>(lo, hi);
        Range<int, 9, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        int count = 0;
        RangeFortranIterationPolicy<9>::forEach(range, [&](const GridType::IndexType& pos){
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
                                                i - lo[0] + (j-lo[1] + (k - lo[2] + (l - lo[3] + (m - lo[4] 
                                                    + (n - lo[5] + (o - lo[6] + (p - lo[7] + (q - lo[8])*dims[7])*dims[6])*dims[5])*dims[4]
                                                )*dims[3])*dims[2])*dims[1])*dims[0]
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

    for (int n=0; n<30; ++n)
    {
        random_extent<10>(lo, hi);
        Range<int, 10, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        int count = 0;
        RangeFortranIterationPolicy<10>::forEach(range, [&](const GridType::IndexType& pos){
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
                                                    i - lo[0] + (j-lo[1] + (k - lo[2] + (l - lo[3] + (m - lo[4] 
                                                        + (n - lo[5] + (o - lo[6] + (p - lo[7] + (q - lo[8] + (r - lo[9])*dims[8])*dims[7])*dims[6])*dims[5])*dims[4]
                                                    )*dims[3])*dims[2])*dims[1])*dims[0]
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