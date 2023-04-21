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


#ifdef KOKKOS_ENABLE_CUDA
    typedef Kokkos::Cuda Execution;
    
    template <typename T, size_t rank>
    using GridStorage = schnek::KokkosGridStorage<T, rank, Kokkos::CudaHostPinnedSpace>;

    typedef schnek::RangeKokkosIterationPolicy<2, Execution> Iteration;
#else
    typedef Kokkos::Serial Execution;

    template <typename T, size_t rank>
    using GridStorage = schnek::SingleArrayGridStorage<T, rank>;

    typedef schnek::RangeCIterationPolicy<2> Iteration;
#endif

struct Assign1d {
    typedef Grid<int, 1, GridNoArgCheck, GridStorage> GridType;
    mutable GridType grid;
    SCHNEK_INLINE void operator()(const GridType::IndexType& pos) const
    {
        grid[pos] = pos[0];
    }
};

struct Assign2d {
    typedef Grid<int, 2, GridNoArgCheck, GridStorage> GridType;
    mutable GridType grid;
    SCHNEK_INLINE void operator()(const GridType::IndexType& pos) const
    {
        grid[pos] = pos[0] + 3*pos[1];
    }
};

struct Assign3d {
    typedef Grid<int, 3, GridNoArgCheck, GridStorage> GridType;
    mutable GridType grid;
    SCHNEK_INLINE void operator()(const GridType::IndexType& pos) const
    {
        grid[pos] = pos[0] + 3*pos[1] + 7*pos[2];
    }
};

struct Assign4d {
    typedef Grid<int, 4, GridNoArgCheck, GridStorage> GridType;
    mutable GridType grid;
    SCHNEK_INLINE void operator()(const GridType::IndexType& pos) const
    {
        grid[pos] = pos[0] + 3*pos[1] + 7*pos[2] + 13*pos[3];
    }
};

struct Assign5d {
    typedef Grid<int, 5, GridNoArgCheck, GridStorage> GridType;
    mutable GridType grid;
    SCHNEK_INLINE void operator()(const GridType::IndexType& pos) const
    {
        grid[pos] = pos[0] + 3*pos[1] + 7*pos[2] + 13*pos[3] + 23*pos[4];
    }
};

struct Assign6d {
    typedef Grid<int, 6, GridNoArgCheck, GridStorage> GridType;
    mutable GridType grid;
    SCHNEK_INLINE void operator()(const GridType::IndexType& pos) const
    {
        grid[pos] = pos[0] + 3*pos[1] + 7*pos[2] + 13*pos[3] + 23*pos[4] + 47*pos[5];
    }
};

BOOST_AUTO_TEST_SUITE( range_iteration )

BOOST_AUTO_TEST_SUITE( kokkos )

BOOST_FIXTURE_TEST_CASE( iterate_1d,  RangeIterationTest)
{
    Assign1d::GridType::IndexType lo(-10), hi(10);

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<10; ++n)
    {
        random_extent<1>(lo, hi);
        Range<int, 1, ArrayNoArgCheck> range(lo, hi);
        Assign1d::GridType grid(lo, hi);
        
        Assign1d assign{grid};
        RangeKokkosIterationPolicy<1, Execution>::forEach(range, assign);
        
        Kokkos::fence();

        for (int i=lo[0]; i<=hi[0]; ++i)
        {
            BOOST_CHECK_EQUAL(grid(i), i);
        }
        ++show_progress;
    }
}


BOOST_FIXTURE_TEST_CASE( iterate_2d, RangeIterationTest )
{
    Assign2d::GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);


    for (int n=0; n<10; ++n)
    {
        random_extent<2>(lo, hi);
        Range<int, 2, ArrayNoArgCheck> range(lo, hi);
        Assign2d::GridType grid(lo, hi);

        Assign2d assign{grid};
        RangeKokkosIterationPolicy<2>::forEach(range, assign);

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

BOOST_FIXTURE_TEST_CASE( iterate_3d, RangeIterationTest )
{
    Assign3d::GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<10; ++n)
    {
        random_extent<3>(lo, hi);
        Range<int, 3, ArrayNoArgCheck> range(lo, hi);
        Assign3d::GridType grid(lo, hi);

        Assign3d assign{grid};
        RangeKokkosIterationPolicy<3>::forEach(range, assign);

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

BOOST_FIXTURE_TEST_CASE( iterate_4d, RangeIterationTest )
{
    Assign4d::GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<10; ++n)
    {
        random_extent<4>(lo, hi);
        Range<int, 4, ArrayNoArgCheck> range(lo, hi);
        Assign4d::GridType grid(lo, hi);

        Assign4d assign{grid};
        RangeKokkosIterationPolicy<4>::forEach(range, assign);

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


BOOST_FIXTURE_TEST_CASE( iterate_5d, RangeIterationTest )
{
    Assign5d::GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<10; ++n)
    {
        random_extent<5>(lo, hi);
        Range<int, 5, ArrayNoArgCheck> range(lo, hi);
        Assign5d::GridType grid(lo, hi);

        Assign5d assign{grid};
        RangeKokkosIterationPolicy<5>::forEach(range, assign);

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


BOOST_FIXTURE_TEST_CASE( iterate_6d, RangeIterationTest )
{
    Assign6d::GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<10; ++n)
    {
        random_extent<6>(lo, hi);
        Range<int, 6, ArrayNoArgCheck> range(lo, hi);
        Assign6d::GridType grid(lo, hi);

        Assign6d assign{grid};
        RangeKokkosIterationPolicy<6>::forEach(range, assign);

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