/*
 * test_array.cpp
 *
 *  Created on: 25 Jul 2017
 *      Author: Holger Schmitz
 */

#include "utility.hpp"

#include <grid/array.hpp>
#include <grid/arrayexpression.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <limits>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/timer/progress_display.hpp>

#include <boost/test/unit_test.hpp>

#pragma GCC diagnostic pop
#pragma GCC diagnostic pop

using namespace schnek;

template<int size>
    struct SizeDummy
{};

struct ArrayTest
{
    boost::random::mt19937 rGen;
    boost::random::uniform_real_distribution<> dist;
    boost::random::uniform_int_distribution<> idist;
    boost::random::uniform_int_distribution<> idist_small;

    ArrayTest()
      : dist(-1.0,1.0),
        idist(std::numeric_limits<int>::min(), std::numeric_limits<int>::max()),
        idist_small(std::numeric_limits<int>::min()/10, std::numeric_limits<int>::max()/10)
    {}

    template<int size>
    void testExpressionAddition(SizeDummy<size>)
    {
      std::vector<int> result(size);

      Array<int, size, ArrayBoostTestArgCheck> A;
      Array<int, size, ArrayBoostTestArgCheck> B;
      Array<int, size, ArrayBoostTestArgCheck> C;
      for (int i=0; i<size; ++i)
      {
        int a = idist_small(rGen);
        int b = idist_small(rGen);
        A[i] = a;
        B[i] = b;
        result[i] = a + b;
      }

      C = A + B;

      for (int i=0; i<size; ++i)
      {
        BOOST_CHECK_EQUAL(C[i], result[i]);
      }
    }

    template<int size>
    void testExpressionSubtraction(SizeDummy<size>)
    {
      std::vector<int> result(size);

      Array<int, size, ArrayBoostTestArgCheck> A;
      Array<int, size, ArrayBoostTestArgCheck> B;
      Array<int, size, ArrayBoostTestArgCheck> C;
      for (int i=0; i<size; ++i)
      {
        int a = idist_small(rGen);
        int b = idist_small(rGen);
        A[i] = a;
        B[i] = b;
        result[i] = a - b;
      }

      C = A - B;

      for (int i=0; i<size; ++i)
      {
        BOOST_CHECK_EQUAL(C[i], result[i]);
      }
    }

    template<int size>
    void testExpressionMultiplication(SizeDummy<size>)
    {
      std::vector<double> result(size);

      double A;
      Array<double, size, ArrayBoostTestArgCheck> B;
      Array<double, size, ArrayBoostTestArgCheck> C;
      Array<double, size, ArrayBoostTestArgCheck> D;
      A = dist(rGen);
      for (int i=0; i<size; ++i)
      {
        double b = dist(rGen);
        B[i] = b;
        result[i] = A*b;
      }

      C = A*B;
      D = B*A;

      for (int i=0; i<size; ++i)
      {
        BOOST_CHECK(is_equal(C[i], result[i]));
        BOOST_CHECK(is_equal(D[i], result[i]));
      }
    }

    template<int size>
    void testExpressionDivision(SizeDummy<size>)
    {
      std::vector<double> result(size);

      double A;
      Array<double, size, ArrayBoostTestArgCheck> B;
      Array<double, size, ArrayBoostTestArgCheck> C;
      A = dist(rGen);
      while (fabs(A)<1e-6) A = dist(rGen);

      for (int i=0; i<size; ++i)
      {
        double b = dist(rGen);
        B[i] = b;
        result[i] = b/A;
      }

      C = B/A;

      for (int i=0; i<size; ++i)
      {
        BOOST_CHECK(is_equal(C[i], result[i]));
      }
    }

    template<int size>
    void testExpressionPrecedence(SizeDummy<size>)
    {
      std::vector<double> result1(size);
      std::vector<double> result2(size);

      double F1;
      double F2;
      Array<double, size, ArrayBoostTestArgCheck> A;
      Array<double, size, ArrayBoostTestArgCheck> B;
      Array<double, size, ArrayBoostTestArgCheck> C;
      Array<double, size, ArrayBoostTestArgCheck> D;

      F1 = dist(rGen);
      F2 = dist(rGen);
      while (fabs(F2)<1e-6) F2 = dist(rGen);

      for (int i=0; i<size; ++i)
      {
        double a = dist(rGen);
        double b = dist(rGen);
        A[i] = a;
        B[i] = b;
        result1[i] = a * F1 + b / F2;
        result2[i] = a *(F1 + b)/ F2;
      }

      C = A * F1 + B / F2;
      D = A *(F1 + B)/ F2;

      for (int i=0; i<size; ++i)
      {
        BOOST_CHECK(is_equal(C[i], result1[i]));
        BOOST_CHECK(is_equal(D[i], result2[i]));
      }
    }

};

BOOST_AUTO_TEST_SUITE( array )

BOOST_FIXTURE_TEST_CASE( constructor_1d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    Array<int, 1, ArrayBoostTestArgCheck> test(v1);
    BOOST_CHECK_EQUAL(v1, test[0]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( constructor_2d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    Array<int, 2, ArrayBoostTestArgCheck> test(v1, v2);
    BOOST_CHECK_EQUAL(v1, test[0]);
    BOOST_CHECK_EQUAL(v2, test[1]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( constructor_3d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    Array<int, 3, ArrayBoostTestArgCheck> test(v1, v2, v3);
    BOOST_CHECK_EQUAL(v1, test[0]);
    BOOST_CHECK_EQUAL(v2, test[1]);
    BOOST_CHECK_EQUAL(v3, test[2]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( constructor_4d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    int v4 = idist(rGen);
    Array<int, 4, ArrayBoostTestArgCheck> test(v1, v2, v3, v4);
    BOOST_CHECK_EQUAL(v1, test[0]);
    BOOST_CHECK_EQUAL(v2, test[1]);
    BOOST_CHECK_EQUAL(v3, test[2]);
    BOOST_CHECK_EQUAL(v4, test[3]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( constructor_5d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    int v4 = idist(rGen);
    int v5 = idist(rGen);
    Array<int, 5, ArrayBoostTestArgCheck> test(v1, v2, v3, v4, v5);
    BOOST_CHECK_EQUAL(v1, test[0]);
    BOOST_CHECK_EQUAL(v2, test[1]);
    BOOST_CHECK_EQUAL(v3, test[2]);
    BOOST_CHECK_EQUAL(v4, test[3]);
    BOOST_CHECK_EQUAL(v5, test[4]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( constructor_6d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    int v4 = idist(rGen);
    int v5 = idist(rGen);
    int v6 = idist(rGen);
    Array<int, 6, ArrayBoostTestArgCheck> test(v1, v2, v3, v4, v5, v6);
    BOOST_CHECK_EQUAL(v1, test[0]);
    BOOST_CHECK_EQUAL(v2, test[1]);
    BOOST_CHECK_EQUAL(v3, test[2]);
    BOOST_CHECK_EQUAL(v4, test[3]);
    BOOST_CHECK_EQUAL(v5, test[4]);
    BOOST_CHECK_EQUAL(v6, test[5]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( constructor_7d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    int v4 = idist(rGen);
    int v5 = idist(rGen);
    int v6 = idist(rGen);
    int v7 = idist(rGen);
    Array<int, 7, ArrayBoostTestArgCheck> test(v1, v2, v3, v4, v5, v6, v7);
    BOOST_CHECK_EQUAL(v1, test[0]);
    BOOST_CHECK_EQUAL(v2, test[1]);
    BOOST_CHECK_EQUAL(v3, test[2]);
    BOOST_CHECK_EQUAL(v4, test[3]);
    BOOST_CHECK_EQUAL(v5, test[4]);
    BOOST_CHECK_EQUAL(v6, test[5]);
    BOOST_CHECK_EQUAL(v7, test[6]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( constructor_8d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    int v4 = idist(rGen);
    int v5 = idist(rGen);
    int v6 = idist(rGen);
    int v7 = idist(rGen);
    int v8 = idist(rGen);
    Array<int, 8, ArrayBoostTestArgCheck> test(v1, v2, v3, v4, v5, v6, v7, v8);
    BOOST_CHECK_EQUAL(v1, test[0]);
    BOOST_CHECK_EQUAL(v2, test[1]);
    BOOST_CHECK_EQUAL(v3, test[2]);
    BOOST_CHECK_EQUAL(v4, test[3]);
    BOOST_CHECK_EQUAL(v5, test[4]);
    BOOST_CHECK_EQUAL(v6, test[5]);
    BOOST_CHECK_EQUAL(v7, test[6]);
    BOOST_CHECK_EQUAL(v8, test[7]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( constructor_9d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    int v4 = idist(rGen);
    int v5 = idist(rGen);
    int v6 = idist(rGen);
    int v7 = idist(rGen);
    int v8 = idist(rGen);
    int v9 = idist(rGen);
    Array<int, 9, ArrayBoostTestArgCheck> test(v1, v2, v3, v4, v5, v6, v7, v8, v9);
    BOOST_CHECK_EQUAL(v1, test[0]);
    BOOST_CHECK_EQUAL(v2, test[1]);
    BOOST_CHECK_EQUAL(v3, test[2]);
    BOOST_CHECK_EQUAL(v4, test[3]);
    BOOST_CHECK_EQUAL(v5, test[4]);
    BOOST_CHECK_EQUAL(v6, test[5]);
    BOOST_CHECK_EQUAL(v7, test[6]);
    BOOST_CHECK_EQUAL(v8, test[7]);
    BOOST_CHECK_EQUAL(v9, test[8]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( constructor_10d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    int v4 = idist(rGen);
    int v5 = idist(rGen);
    int v6 = idist(rGen);
    int v7 = idist(rGen);
    int v8 = idist(rGen);
    int v9 = idist(rGen);
    int v10 = idist(rGen);
    Array<int, 10, ArrayBoostTestArgCheck> test(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
    BOOST_CHECK_EQUAL(v1, test[0]);
    BOOST_CHECK_EQUAL(v2, test[1]);
    BOOST_CHECK_EQUAL(v3, test[2]);
    BOOST_CHECK_EQUAL(v4, test[3]);
    BOOST_CHECK_EQUAL(v5, test[4]);
    BOOST_CHECK_EQUAL(v6, test[5]);
    BOOST_CHECK_EQUAL(v7, test[6]);
    BOOST_CHECK_EQUAL(v8, test[7]);
    BOOST_CHECK_EQUAL(v9, test[8]);
    BOOST_CHECK_EQUAL(v10, test[9]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_addition_1d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionAddition(SizeDummy<1>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_addition_2d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionAddition(SizeDummy<2>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_addition_3d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionAddition(SizeDummy<3>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_addition_4d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionAddition(SizeDummy<4>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_addition_5d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionAddition(SizeDummy<5>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_addition_6d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionAddition(SizeDummy<6>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_addition_7d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionAddition(SizeDummy<7>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_addition_8d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionAddition(SizeDummy<8>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_addition_9d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionAddition(SizeDummy<9>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_addition_10d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionAddition(SizeDummy<10>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_subtraction_1d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionSubtraction(SizeDummy<1>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_subtraction_2d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionSubtraction(SizeDummy<2>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_subtraction_3d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionSubtraction(SizeDummy<3>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_subtraction_4d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionSubtraction(SizeDummy<4>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_subtraction_5d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionSubtraction(SizeDummy<5>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_subtraction_6d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionSubtraction(SizeDummy<6>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_subtraction_7d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionSubtraction(SizeDummy<7>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_subtraction_8d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionSubtraction(SizeDummy<8>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_subtraction_9d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionSubtraction(SizeDummy<9>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_subtraction_10d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionSubtraction(SizeDummy<10>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_multiplication_1d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionMultiplication(SizeDummy<1>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_multiplication_2d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionMultiplication(SizeDummy<2>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_multiplication_3d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionMultiplication(SizeDummy<3>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_multiplication_4d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionMultiplication(SizeDummy<4>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_multiplication_5d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionMultiplication(SizeDummy<5>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_multiplication_6d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionMultiplication(SizeDummy<6>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_multiplication_7d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionMultiplication(SizeDummy<7>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_multiplication_8d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionMultiplication(SizeDummy<8>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_multiplication_9d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionMultiplication(SizeDummy<9>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_multiplication_10d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionMultiplication(SizeDummy<10>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_division_1d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionDivision(SizeDummy<1>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_division_2d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionDivision(SizeDummy<2>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_division_3d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionDivision(SizeDummy<3>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_division_4d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionDivision(SizeDummy<4>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_division_5d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionDivision(SizeDummy<5>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_division_6d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionDivision(SizeDummy<6>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_division_7d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionDivision(SizeDummy<7>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_division_8d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionDivision(SizeDummy<8>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_division_9d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionDivision(SizeDummy<9>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_division_10d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionDivision(SizeDummy<10>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_precedence_1d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionPrecedence(SizeDummy<1>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_precedence_2d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionPrecedence(SizeDummy<2>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_precedence_3d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionPrecedence(SizeDummy<3>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_precedence_4d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionPrecedence(SizeDummy<4>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_precedence_5d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionPrecedence(SizeDummy<5>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_precedence_6d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionPrecedence(SizeDummy<6>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_precedence_7d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionPrecedence(SizeDummy<7>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_precedence_8d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionPrecedence(SizeDummy<8>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_precedence_9d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionPrecedence(SizeDummy<9>());
    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( expression_precedence_10d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    testExpressionPrecedence(SizeDummy<10>());
    ++show_progress;
  }
}


BOOST_FIXTURE_TEST_CASE( project_2d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    Array<int, 2, ArrayBoostTestArgCheck> test(v1, v2);
    Array<int, 1, ArrayBoostTestArgCheck> testProject1 = test.project<1>();

    BOOST_CHECK_EQUAL(testProject1[0], test[0]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( project_3d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    Array<int, 3, ArrayBoostTestArgCheck> test(v1, v2, v3);

    Array<int, 1, ArrayBoostTestArgCheck> testProject1 = test.project<1>();
    Array<int, 2, ArrayBoostTestArgCheck> testProject2 = test.project<2>();

    BOOST_CHECK_EQUAL(testProject1[0], test[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2[i], test[i]);

    ++show_progress;
  }
}


BOOST_FIXTURE_TEST_CASE( project_4d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    int v4 = idist(rGen);
    Array<int, 4, ArrayBoostTestArgCheck> test(v1, v2, v3, v4);

    Array<int, 1, ArrayBoostTestArgCheck> testProject1 = test.project<1>();
    Array<int, 2, ArrayBoostTestArgCheck> testProject2 = test.project<2>();
    Array<int, 3, ArrayBoostTestArgCheck> testProject3 = test.project<3>();

    BOOST_CHECK_EQUAL(testProject1[0], test[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2[i], test[i]);
    for (int i=0; i<3; ++i) BOOST_CHECK_EQUAL(testProject3[i], test[i]);

    ++show_progress;
  }
}


BOOST_FIXTURE_TEST_CASE( project_5d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    int v4 = idist(rGen);
    int v5 = idist(rGen);
    Array<int, 5, ArrayBoostTestArgCheck> test(v1, v2, v3, v4, v5);

    Array<int, 1, ArrayBoostTestArgCheck> testProject1 = test.project<1>();
    Array<int, 2, ArrayBoostTestArgCheck> testProject2 = test.project<2>();
    Array<int, 3, ArrayBoostTestArgCheck> testProject3 = test.project<3>();
    Array<int, 4, ArrayBoostTestArgCheck> testProject4 = test.project<4>();

    BOOST_CHECK_EQUAL(testProject1[0], test[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2[i], test[i]);
    for (int i=0; i<3; ++i) BOOST_CHECK_EQUAL(testProject3[i], test[i]);
    for (int i=0; i<4; ++i) BOOST_CHECK_EQUAL(testProject4[i], test[i]);

    ++show_progress;
  }
}


BOOST_FIXTURE_TEST_CASE( project_6d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    int v4 = idist(rGen);
    int v5 = idist(rGen);
    int v6 = idist(rGen);
    Array<int, 6, ArrayBoostTestArgCheck> test(v1, v2, v3, v4, v5, v6);

    Array<int, 1, ArrayBoostTestArgCheck> testProject1 = test.project<1>();
    Array<int, 2, ArrayBoostTestArgCheck> testProject2 = test.project<2>();
    Array<int, 3, ArrayBoostTestArgCheck> testProject3 = test.project<3>();
    Array<int, 4, ArrayBoostTestArgCheck> testProject4 = test.project<4>();
    Array<int, 5, ArrayBoostTestArgCheck> testProject5 = test.project<5>();

    BOOST_CHECK_EQUAL(testProject1[0], test[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2[i], test[i]);
    for (int i=0; i<3; ++i) BOOST_CHECK_EQUAL(testProject3[i], test[i]);
    for (int i=0; i<4; ++i) BOOST_CHECK_EQUAL(testProject4[i], test[i]);
    for (int i=0; i<5; ++i) BOOST_CHECK_EQUAL(testProject5[i], test[i]);

    ++show_progress;
  }
}


BOOST_FIXTURE_TEST_CASE( project_7d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    int v4 = idist(rGen);
    int v5 = idist(rGen);
    int v6 = idist(rGen);
    int v7 = idist(rGen);
    Array<int, 7, ArrayBoostTestArgCheck> test(v1, v2, v3, v4, v5, v6, v7);

    Array<int, 1, ArrayBoostTestArgCheck> testProject1 = test.project<1>();
    Array<int, 2, ArrayBoostTestArgCheck> testProject2 = test.project<2>();
    Array<int, 3, ArrayBoostTestArgCheck> testProject3 = test.project<3>();
    Array<int, 4, ArrayBoostTestArgCheck> testProject4 = test.project<4>();
    Array<int, 5, ArrayBoostTestArgCheck> testProject5 = test.project<5>();
    Array<int, 6, ArrayBoostTestArgCheck> testProject6 = test.project<6>();

    BOOST_CHECK_EQUAL(testProject1[0], test[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2[i], test[i]);
    for (int i=0; i<3; ++i) BOOST_CHECK_EQUAL(testProject3[i], test[i]);
    for (int i=0; i<4; ++i) BOOST_CHECK_EQUAL(testProject4[i], test[i]);
    for (int i=0; i<5; ++i) BOOST_CHECK_EQUAL(testProject5[i], test[i]);
    for (int i=0; i<6; ++i) BOOST_CHECK_EQUAL(testProject6[i], test[i]);

    ++show_progress;
  }
}


BOOST_FIXTURE_TEST_CASE( project_8d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    int v4 = idist(rGen);
    int v5 = idist(rGen);
    int v6 = idist(rGen);
    int v7 = idist(rGen);
    int v8 = idist(rGen);
    Array<int, 8, ArrayBoostTestArgCheck> test(v1, v2, v3, v4, v5, v6, v7, v8);

    Array<int, 1, ArrayBoostTestArgCheck> testProject1 = test.project<1>();
    Array<int, 2, ArrayBoostTestArgCheck> testProject2 = test.project<2>();
    Array<int, 3, ArrayBoostTestArgCheck> testProject3 = test.project<3>();
    Array<int, 4, ArrayBoostTestArgCheck> testProject4 = test.project<4>();
    Array<int, 5, ArrayBoostTestArgCheck> testProject5 = test.project<5>();
    Array<int, 6, ArrayBoostTestArgCheck> testProject6 = test.project<6>();
    Array<int, 7, ArrayBoostTestArgCheck> testProject7 = test.project<7>();

    BOOST_CHECK_EQUAL(testProject1[0], test[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2[i], test[i]);
    for (int i=0; i<3; ++i) BOOST_CHECK_EQUAL(testProject3[i], test[i]);
    for (int i=0; i<4; ++i) BOOST_CHECK_EQUAL(testProject4[i], test[i]);
    for (int i=0; i<5; ++i) BOOST_CHECK_EQUAL(testProject5[i], test[i]);
    for (int i=0; i<6; ++i) BOOST_CHECK_EQUAL(testProject6[i], test[i]);
    for (int i=0; i<7; ++i) BOOST_CHECK_EQUAL(testProject7[i], test[i]);

    ++show_progress;
  }
}


BOOST_FIXTURE_TEST_CASE( project_9d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    int v4 = idist(rGen);
    int v5 = idist(rGen);
    int v6 = idist(rGen);
    int v7 = idist(rGen);
    int v8 = idist(rGen);
    int v9 = idist(rGen);
    Array<int, 9, ArrayBoostTestArgCheck> test(v1, v2, v3, v4, v5, v6, v7, v8, v9);

    Array<int, 1, ArrayBoostTestArgCheck> testProject1 = test.project<1>();
    Array<int, 2, ArrayBoostTestArgCheck> testProject2 = test.project<2>();
    Array<int, 3, ArrayBoostTestArgCheck> testProject3 = test.project<3>();
    Array<int, 4, ArrayBoostTestArgCheck> testProject4 = test.project<4>();
    Array<int, 5, ArrayBoostTestArgCheck> testProject5 = test.project<5>();
    Array<int, 6, ArrayBoostTestArgCheck> testProject6 = test.project<6>();
    Array<int, 7, ArrayBoostTestArgCheck> testProject7 = test.project<7>();
    Array<int, 8, ArrayBoostTestArgCheck> testProject8 = test.project<8>();

    BOOST_CHECK_EQUAL(testProject1[0], test[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2[i], test[i]);
    for (int i=0; i<3; ++i) BOOST_CHECK_EQUAL(testProject3[i], test[i]);
    for (int i=0; i<4; ++i) BOOST_CHECK_EQUAL(testProject4[i], test[i]);
    for (int i=0; i<5; ++i) BOOST_CHECK_EQUAL(testProject5[i], test[i]);
    for (int i=0; i<6; ++i) BOOST_CHECK_EQUAL(testProject6[i], test[i]);
    for (int i=0; i<7; ++i) BOOST_CHECK_EQUAL(testProject7[i], test[i]);
    for (int i=0; i<8; ++i) BOOST_CHECK_EQUAL(testProject8[i], test[i]);

    ++show_progress;
  }
}


BOOST_FIXTURE_TEST_CASE( project_10d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    int v4 = idist(rGen);
    int v5 = idist(rGen);
    int v6 = idist(rGen);
    int v7 = idist(rGen);
    int v8 = idist(rGen);
    int v9 = idist(rGen);
    int v10 = idist(rGen);
    Array<int, 10, ArrayBoostTestArgCheck> test(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);

    Array<int, 1, ArrayBoostTestArgCheck> testProject1 = test.project<1>();
    Array<int, 2, ArrayBoostTestArgCheck> testProject2 = test.project<2>();
    Array<int, 3, ArrayBoostTestArgCheck> testProject3 = test.project<3>();
    Array<int, 4, ArrayBoostTestArgCheck> testProject4 = test.project<4>();
    Array<int, 5, ArrayBoostTestArgCheck> testProject5 = test.project<5>();
    Array<int, 6, ArrayBoostTestArgCheck> testProject6 = test.project<6>();
    Array<int, 7, ArrayBoostTestArgCheck> testProject7 = test.project<7>();
    Array<int, 8, ArrayBoostTestArgCheck> testProject8 = test.project<8>();
    Array<int, 9, ArrayBoostTestArgCheck> testProject9 = test.project<9>();

    BOOST_CHECK_EQUAL(testProject1[0], test[0]);
    for (int i=0; i<2; ++i) BOOST_CHECK_EQUAL(testProject2[i], test[i]);
    for (int i=0; i<3; ++i) BOOST_CHECK_EQUAL(testProject3[i], test[i]);
    for (int i=0; i<4; ++i) BOOST_CHECK_EQUAL(testProject4[i], test[i]);
    for (int i=0; i<5; ++i) BOOST_CHECK_EQUAL(testProject5[i], test[i]);
    for (int i=0; i<6; ++i) BOOST_CHECK_EQUAL(testProject6[i], test[i]);
    for (int i=0; i<7; ++i) BOOST_CHECK_EQUAL(testProject7[i], test[i]);
    for (int i=0; i<8; ++i) BOOST_CHECK_EQUAL(testProject8[i], test[i]);
    for (int i=0; i<9; ++i) BOOST_CHECK_EQUAL(testProject9[i], test[i]);

    ++show_progress;
  }
}

BOOST_FIXTURE_TEST_CASE( projectDim_2d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);
  boost::random::uniform_int_distribution<> index_dist(0, 1);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int dim = index_dist(rGen);
    std::vector<int> indices(2);
    indices[0] = 0;
    indices[1] = 1;
    indices.erase(indices.begin()+dim);
    Array<int, 2, ArrayBoostTestArgCheck> test(v1, v2);
    Array<int, 1, ArrayBoostTestArgCheck> test_projectDim = test.projectDim(dim);
    BOOST_CHECK_EQUAL(test_projectDim[0], test[indices[0]]);

    ++show_progress;
  }
}


BOOST_FIXTURE_TEST_CASE( projectDim_3d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);
  boost::random::uniform_int_distribution<> index_dist(0, 2);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    int dim = index_dist(rGen);

    std::vector<int> indices(3);
    for (int i=0; i<3; ++i) indices[i] = i;
    indices.erase(indices.begin()+dim);

    Array<int, 3, ArrayBoostTestArgCheck> test(v1, v2, v3);
    Array<int, 2, ArrayBoostTestArgCheck> test_projectDim = test.projectDim(dim);
    for (int i=0; i<2; ++i)
      BOOST_CHECK_EQUAL(test_projectDim[i], test[indices[i]]);

    ++show_progress;
  }
}


BOOST_FIXTURE_TEST_CASE( projectDim_4d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);
  boost::random::uniform_int_distribution<> index_dist(0, 3);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    int v4 = idist(rGen);
    int dim = index_dist(rGen);

    std::vector<int> indices(4);
    for (int i=0; i<4; ++i) indices[i] = i;
    indices.erase(indices.begin()+dim);

    Array<int, 4, ArrayBoostTestArgCheck> test(v1, v2, v3, v4);
    Array<int, 3, ArrayBoostTestArgCheck> test_projectDim = test.projectDim(dim);
    for (int i=0; i<3; ++i)
      BOOST_CHECK_EQUAL(test_projectDim[i], test[indices[i]]);

    ++show_progress;
  }
}


BOOST_FIXTURE_TEST_CASE( projectDim_5d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);
  boost::random::uniform_int_distribution<> index_dist(0, 4);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    int v4 = idist(rGen);
    int v5 = idist(rGen);
    int dim = index_dist(rGen);

    std::vector<int> indices(5);
    for (int i=0; i<5; ++i) indices[i] = i;
    indices.erase(indices.begin()+dim);

    Array<int, 5, ArrayBoostTestArgCheck> test(v1, v2, v3, v4, v5);
    Array<int, 4, ArrayBoostTestArgCheck> test_projectDim = test.projectDim(dim);
    for (int i=0; i<4; ++i)
      BOOST_CHECK_EQUAL(test_projectDim[i], test[indices[i]]);

    ++show_progress;
  }
}


BOOST_FIXTURE_TEST_CASE( projectDim_6d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);
  boost::random::uniform_int_distribution<> index_dist(0, 5);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    int v4 = idist(rGen);
    int v5 = idist(rGen);
    int v6 = idist(rGen);
    int dim = index_dist(rGen);

    std::vector<int> indices(6);
    for (int i=0; i<6; ++i) indices[i] = i;
    indices.erase(indices.begin()+dim);

    Array<int, 6, ArrayBoostTestArgCheck> test(v1, v2, v3, v4, v5, v6);
    Array<int, 5, ArrayBoostTestArgCheck> test_projectDim = test.projectDim(dim);
    for (int i=0; i<5; ++i)
      BOOST_CHECK_EQUAL(test_projectDim[i], test[indices[i]]);

    ++show_progress;
  }
}


BOOST_FIXTURE_TEST_CASE( projectDim_7d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);
  boost::random::uniform_int_distribution<> index_dist(0, 6);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    int v4 = idist(rGen);
    int v5 = idist(rGen);
    int v6 = idist(rGen);
    int v7 = idist(rGen);
    int dim = index_dist(rGen);

    std::vector<int> indices(7);
    for (int i=0; i<7; ++i) indices[i] = i;
    indices.erase(indices.begin()+dim);

    Array<int, 7, ArrayBoostTestArgCheck> test(v1, v2, v3, v4, v5, v6, v7);
    Array<int, 6, ArrayBoostTestArgCheck> test_projectDim = test.projectDim(dim);
    for (int i=0; i<6; ++i)
      BOOST_CHECK_EQUAL(test_projectDim[i], test[indices[i]]);

    ++show_progress;
  }
}


BOOST_FIXTURE_TEST_CASE( projectDim_8d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);
  boost::random::uniform_int_distribution<> index_dist(0, 7);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    int v4 = idist(rGen);
    int v5 = idist(rGen);
    int v6 = idist(rGen);
    int v7 = idist(rGen);
    int v8 = idist(rGen);
    int dim = index_dist(rGen);

    std::vector<int> indices(8);
    for (int i=0; i<8; ++i) indices[i] = i;
    indices.erase(indices.begin()+dim);

    Array<int, 8, ArrayBoostTestArgCheck> test(v1, v2, v3, v4, v5, v6, v7, v8);
    Array<int, 7, ArrayBoostTestArgCheck> test_projectDim = test.projectDim(dim);
    for (int i=0; i<7; ++i)
      BOOST_CHECK_EQUAL(test_projectDim[i], test[indices[i]]);

    ++show_progress;
  }
}


BOOST_FIXTURE_TEST_CASE( projectDim_9d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);
  boost::random::uniform_int_distribution<> index_dist(0, 8);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    int v4 = idist(rGen);
    int v5 = idist(rGen);
    int v6 = idist(rGen);
    int v7 = idist(rGen);
    int v8 = idist(rGen);
    int v9 = idist(rGen);
    int dim = index_dist(rGen);

    std::vector<int> indices(9);
    for (int i=0; i<9; ++i) indices[i] = i;
    indices.erase(indices.begin()+dim);

    Array<int, 9, ArrayBoostTestArgCheck> test(v1, v2, v3, v4, v5, v6, v7, v8, v9);
    Array<int, 8, ArrayBoostTestArgCheck> test_projectDim = test.projectDim(dim);
    for (int i=0; i<8; ++i)
      BOOST_CHECK_EQUAL(test_projectDim[i], test[indices[i]]);

    ++show_progress;
  }
}


BOOST_FIXTURE_TEST_CASE( projectDim_10d, ArrayTest )
{
  const int N = 100000;
  boost::timer::progress_display show_progress(N);
  boost::random::uniform_int_distribution<> index_dist(0, 9);

  for (int i=0; i<N; i++) {
    int v1 = idist(rGen);
    int v2 = idist(rGen);
    int v3 = idist(rGen);
    int v4 = idist(rGen);
    int v5 = idist(rGen);
    int v6 = idist(rGen);
    int v7 = idist(rGen);
    int v8 = idist(rGen);
    int v9 = idist(rGen);
    int v10 = idist(rGen);
    int dim = index_dist(rGen);

    std::vector<int> indices(10);
    for (int i=0; i<10; ++i) indices[i] = i;
    indices.erase(indices.begin()+dim);

    Array<int, 10, ArrayBoostTestArgCheck> test(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
    Array<int, 9, ArrayBoostTestArgCheck> test_projectDim = test.projectDim(dim);
    for (int i=0; i<9; ++i)
      BOOST_CHECK_EQUAL(test_projectDim[i], test[indices[i]]);

    ++show_progress;
  }
}

BOOST_AUTO_TEST_SUITE_END()

