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

// #include <schnek/util/logger.hpp>
// #include <schnek/grid/iteration/range-iteration.hpp>
// #include <schnek/grid/iteration/kokkos-iteration.hpp>
// #include <schnek/grid/gridstorage/kokkos-storage.hpp>
// #include <schnek/grid/gridstorage.hpp>
// #include <schnek/grid/grid.hpp>

// #include <Kokkos_Core.hpp>

// // #include <string>
// #include <chrono>
// #include <cassert>

// #ifdef KOKKOS_ENABLE_CUDA
//     typedef Kokkos::Cuda Execution;
//     typedef Kokkos::CudaSpace Memory;

//     template <typename T, size_t rank>
//     using GridStorage = schnek::KokkosGridStorage<T, rank, Memory>;
    
//     template <typename T, size_t rank>
//     using GridStoragePinned = schnek::KokkosGridStorage<T, rank, Kokkos::CudaHostPinnedSpace>;

//     typedef schnek::RangeKokkosIterationPolicy<2, Execution> Iteration;
// #else
//     typedef Kokkos::Serial Execution;
//     typedef Kokkos::HostSpace Memory;

//     template <typename T, size_t rank>
//     using GridStorage = schnek::SingleArrayGridStorage<T, rank>;
//     template <typename T, size_t rank>
//     using GridStoragePinned = schnek::SingleArrayGridStorage<T, rank>;

//     typedef schnek::RangeCIterationPolicy<2> Iteration;
// #endif

// typedef schnek::Grid<double, 2, schnek::GridNoArgCheck, GridStorage> Grid;
// typedef schnek::Grid<double, 2, schnek::GridNoArgCheck, GridStoragePinned> GridPinned;

// typedef schnek::Array<int, 2> Index;
// typedef schnek::Range<int, 2> Range;

// Range gridRange = Range(Index(0, 0), Index(1000, 1000));

// template<typename GridTo = Grid, typename GridFrom = Grid>
// struct CopyGrid
// {
//     mutable GridTo to;
//     GridFrom from;
//     KOKKOS_INLINE_FUNCTION void operator()(Index i) const
//     {
//         int x = i[0];
//         int y = i[1];
//         to(x,y) = from(x,y);
//     }
// };

// void perform_calculations()
// {   
//     Grid in(gridRange.getLo(), gridRange.getHi());
//     GridPinned pinned(gridRange.getLo(), gridRange.getHi());

//     for (int i=0; i<=1000; ++i) {
//         for (int j=0; j<=1000; ++j) {
//             pinned(i, j) = 0;
//         }
//     }
//     pinned(500, 500) = 1;
    
//     CopyGrid<Grid, GridPinned> toHost{in, pinned};

//     Iteration::forEach(gridRange, toHost);
//     Kokkos::fence();

// }


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

struct Assign1d {
    typedef Grid<int, 1, GridNoArgCheck, schnek::KokkosDefaultGridStorage> GridType;
    mutable GridType grid;
    SCHNEK_INLINE void operator()(const GridType::IndexType& pos) const
    {
        grid[pos] = pos[0];
    }
};

struct Assign2d {
    typedef Grid<int, 2, GridNoArgCheck, schnek::KokkosDefaultGridStorage> GridType;
    mutable GridType grid;
    SCHNEK_INLINE void operator()(const GridType::IndexType& pos) const
    {
        grid[pos] = pos[0] + 3*pos[1];
    }
};

struct Assign3d {
    typedef Grid<int, 3, GridNoArgCheck, schnek::KokkosDefaultGridStorage> GridType;
    mutable GridType grid;
    SCHNEK_INLINE void operator()(const GridType::IndexType& pos) const
    {
        grid[pos] = pos[0] + 3*pos[1] + 7*pos[2];
    }
};

struct Assign4d {
    typedef Grid<int, 4, GridNoArgCheck, schnek::KokkosDefaultGridStorage> GridType;
    mutable GridType grid;
    SCHNEK_INLINE void operator()(const GridType::IndexType& pos) const
    {
        grid[pos] = pos[0] + 3*pos[1] + 7*pos[2] + 13*pos[3];
    }
};

struct Assign5d {
    typedef Grid<int, 5, GridNoArgCheck, schnek::KokkosDefaultGridStorage> GridType;
    mutable GridType grid;
    SCHNEK_INLINE void operator()(const GridType::IndexType& pos) const
    {
        grid[pos] = pos[0] + 3*pos[1] + 7*pos[2] + 13*pos[3] + 23*pos[4];
    }
};

struct Assign6d {
    typedef Grid<int, 6, GridNoArgCheck, schnek::KokkosDefaultGridStorage> GridType;
    mutable GridType grid;
    SCHNEK_INLINE void operator()(const GridType::IndexType& pos) const
    {
        grid[pos] = pos[0] + 3*pos[1] + 7*pos[2] + 13*pos[3] + 23*pos[4] + 47*pos[5];
    }
};

BOOST_AUTO_TEST_SUITE( range_iteration )

BOOST_AUTO_TEST_SUITE( kokkos )

BOOST_FIXTURE_TEST_CASE( iterate_1d,  KokkosIterationTest)
{
    Assign1d::GridType::IndexType lo(-10), hi(10);

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<30; ++n)
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


BOOST_FIXTURE_TEST_CASE( iterate_2d, KokkosIterationTest )
{
    Assign2d::GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);


    for (int n=0; n<30; ++n)
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

BOOST_FIXTURE_TEST_CASE( iterate_3d, KokkosIterationTest )
{
    Assign3d::GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<30; ++n)
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

BOOST_FIXTURE_TEST_CASE( iterate_4d, KokkosIterationTest )
{
    Assign4d::GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<30; ++n)
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


BOOST_FIXTURE_TEST_CASE( iterate_5d, KokkosIterationTest )
{
    Assign5d::GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<30; ++n)
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


BOOST_FIXTURE_TEST_CASE( iterate_6d, KokkosIterationTest )
{
    Assign6d::GridType::IndexType lo, hi;

    boost::timer::progress_display show_progress(30);

    for (int n=0; n<30; ++n)
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