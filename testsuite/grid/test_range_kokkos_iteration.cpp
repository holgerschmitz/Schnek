/*
 * test_range_kokkos_iteration.cpp
 *
 *  Created on: 16 Dec 2022
 *      Author: Holger Schmitz
 */

#include "../utility.hpp"
#include "range_test_fixture.hpp"

#include <grid/iteration/kokkos-iteration.hpp>
#include <grid/gridstorage/kokkos-storage.hpp>
#include <grid/grid.hpp>
#include <grid/range.hpp>

#include <boost/timer/progress_display.hpp>

#include <boost/test/unit_test.hpp>

using namespace schnek;

#ifdef SCHNEK_HAVE_KOKKOS

struct KokkosIterationTest : public RangeIterationTest
{
    KokkosIterationTest() : RangeIterationTest() {
        Kokkos::InitArguments args;
        args.num_threads = 0;
        args.num_numa = 0;
        Kokkos::initialize(args);
    }

    ~KokkosIterationTest() {
        Kokkos::finalize();
    }
};

BOOST_AUTO_TEST_SUITE( range_iteration )

BOOST_AUTO_TEST_SUITE( kokkos )

BOOST_FIXTURE_TEST_CASE( iterate_1d,  KokkosIterationTest)
{
    typedef Grid<int, 1, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;

    GridType::IndexType lo(-10), hi(10);

    boost::timer::progress_display show_progress(30);
    for (int n=0; n<30; ++n)
    {
        random_extent<1>(lo, hi);
        Range<int, 1, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);

        RangeKokkosIterationPolicy<1>::forEach(range, [&](const GridType::IndexType& pos){
            grid[pos] = pos[0];
        });
        
        Kokkos::fence();

        for (int i=lo[0]; i<=hi[0]; ++i)
        {
            BOOST_CHECK_EQUAL(grid(i), i);
        }
        ++show_progress;
    }
}

BOOST_FIXTURE_TEST_CASE( iterate_2d, KokkosIterationTest )
{
    typedef Grid<int, 2, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;

    GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<30; ++n)
    {
        random_extent<2>(lo, hi);
        Range<int, 2, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        RangeKokkosIterationPolicy<2>::forEach(range, [&](const GridType::IndexType& pos){
            grid[pos] = pos[0] + 3*pos[1];
        });

        Kokkos::fence();

        for (int i=lo[0]; i<=hi[0]; ++i)
        {
            for (int j=lo[1]; j<=hi[1]; ++j)
            {
                BOOST_CHECK_EQUAL(grid(i, j), i + 3*j);
            }
        }
        ++show_progress;
    }
}

BOOST_FIXTURE_TEST_CASE( iterate_3d, KokkosIterationTest )
{
    typedef Grid<int, 3, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;

    GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<30; ++n)
    {
        random_extent<3>(lo, hi);
        Range<int, 3, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        RangeKokkosIterationPolicy<3>::forEach(range, [&](const GridType::IndexType& pos){
            grid[pos] = pos[0] + 3*pos[1] + 7*pos[2];
        });

        Kokkos::fence();

        for (int i=lo[0]; i<=hi[0]; ++i)
        {
            for (int j=lo[1]; j<=hi[1]; ++j)
            {
                for (int k=lo[2]; k<=hi[2]; ++k)
                {
                    BOOST_CHECK_EQUAL(grid(i, j, k), i + 3*j + 7*k);
                }
            }
        }
        ++show_progress;
    }
}

BOOST_FIXTURE_TEST_CASE( iterate_4d, KokkosIterationTest )
{
    typedef Grid<int, 4, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;

    GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<30; ++n)
    {
        random_extent<4>(lo, hi);
        Range<int, 4, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        RangeKokkosIterationPolicy<4>::forEach(range, [&](const GridType::IndexType& pos){
            grid[pos] = pos[0] + 3*pos[1] + 7*pos[2] + 13*pos[3];
        });

        Kokkos::fence();

        for (int i=lo[0]; i<=hi[0]; ++i)
        {
            for (int j=lo[1]; j<=hi[1]; ++j)
            {
                for (int k=lo[2]; k<=hi[2]; ++k)
                {
                    for (int l=lo[3]; l<=hi[3]; ++l)
                    {
                        BOOST_CHECK_EQUAL(grid(i, j, k, l), 
                            i + 3*j + 7*k + 13*l
                        );
                    }
                }
            }
        }
        ++show_progress;
    }
}


BOOST_FIXTURE_TEST_CASE( iterate_5d, KokkosIterationTest )
{
    typedef Grid<int, 5, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;

    GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<30; ++n)
    {
        random_extent<5>(lo, hi);
        Range<int, 5, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        RangeKokkosIterationPolicy<5>::forEach(range, [&](const GridType::IndexType& pos){
            grid[pos] = pos[0] + 3*pos[1] + 7*pos[2] + 13*pos[3] + 23*pos[4];
        });

        Kokkos::fence();

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
                                i + 3*j + 7*k + 13*l + 23*m
                            );
                        }
                    }
                }
            }
        }
        ++show_progress;
    }
}


BOOST_FIXTURE_TEST_CASE( iterate_6d, KokkosIterationTest )
{
    typedef Grid<int, 6, GridBoostTestCheck, schnek::KokkosDefaultGridStorage> GridType;

    GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<30; ++n)
    {
        random_extent<6>(lo, hi);
        Range<int, 6, ArrayBoostTestArgCheck> range(lo, hi);
        GridType grid(lo, hi);
        auto dims = grid.getDims();

        RangeKokkosIterationPolicy<6>::forEach(range, [&](const GridType::IndexType& pos){
            grid[pos] = pos[0] + 3*pos[1] + 7*pos[2] + 13*pos[3] + 23*pos[4] + 47*pos[5];
        });

        Kokkos::fence();

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
                                    i + 3*j + 7*k + 13*l + 23*m + 47*n
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

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

#endif