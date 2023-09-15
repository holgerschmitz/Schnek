
#include <computation/algorithm.hpp>
#include <grid/field.hpp>
#include <grid/grid.hpp>
#include <grid/gridstorage.hpp>

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
    ::MultiArchitectureFieldFactory<schnek::computation::FieldTypeWrapper<int, 2>::type> factory;
  // factory::create<TestArchitecture>({{0,0}, {100,100}}, {{0,0}, {1,1}}, {false, false}, 2);
}


BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()