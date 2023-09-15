
#include <computation/algorithm.hpp>
#include <grid/field.hpp>
#include <grid/grid.hpp>
#include <grid/gridstorage.hpp>
#include <grid/range.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( computation )
BOOST_AUTO_TEST_SUITE( algorithm )

struct TestArchitecture {
    template<typename T, size_t rank>
    using GridStorageType = schnek::SingleArrayGridStorage<T, rank>;
};

BOOST_AUTO_TEST_CASE( MultiArchitectureFieldFactory )
{
  schnek::computation
    ::MultiArchitectureFieldFactory< schnek::computation::FieldTypeWrapper<int, 2> > factory;

  auto field = factory.create<TestArchitecture>(
    schnek::Range<int, 2>(schnek::Array<int, 2>(0, 10), schnek::Array<int, 2>(100, 150)), 
    schnek::Range<double, 2>(schnek::Array<double, 2>(0, 0.5), schnek::Array<double, 2>(1, 1.25)), 
    schnek::Array<bool, 2>(false, false), 
    2
  );

  BOOST_CHECK_EQUAL(field.getLo(0), -2);
  BOOST_CHECK_EQUAL(field.getHi(0), 102);
  BOOST_CHECK_EQUAL(field.getLo(1), 8);
  BOOST_CHECK_EQUAL(field.getHi(1), 152);

  BOOST_CHECK_EQUAL(field.getInnerLo()[0], 0);
  BOOST_CHECK_EQUAL(field.getInnerHi()[0], 100);
  BOOST_CHECK_EQUAL(field.getInnerLo()[1], 10);
  BOOST_CHECK_EQUAL(field.getInnerHi()[1], 150);

  auto domain = field.getDomain();
  BOOST_CHECK_EQUAL(domain.getLo(0), 0);
  BOOST_CHECK_EQUAL(domain.getHi(0), 1);
  BOOST_CHECK_EQUAL(domain.getLo(1), 0.5);
  BOOST_CHECK_EQUAL(domain.getHi(1), 1.25);

}


BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()