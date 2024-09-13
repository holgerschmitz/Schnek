
#include <computation/field-factory.hpp>
#include <computation/algorithm.hpp>
#include <generic/type-util.hpp>

#include <grid/field.hpp>
#include <grid/grid.hpp>
#include <grid/gridstorage.hpp>
#include <grid/range.hpp>

#include <boost/test/unit_test.hpp>

#include <string>

BOOST_AUTO_TEST_SUITE( computation )
BOOST_AUTO_TEST_SUITE( algorithm )

struct TestArchitecture;
struct TestArchitecture2;
struct TestArchitecture3;

struct TestArchitecture {
    template<typename T, size_t rank>
    using GridStorageType = schnek::SingleArrayGridStorage<T, rank>;

    static const std::string id;
    
    typedef schnek::generic::TypeList<TestArchitecture2> AllowedCopySources;
};

struct TestArchitecture2 {
    template<typename T, size_t rank>
    using GridStorageType = schnek::SingleArrayGridStorageFortran<T, rank>;

    static const std::string id;

    typedef TestArchitecture PreferredCopySource;
    typedef schnek::generic::TypeList<TestArchitecture> AllowedCopySources;
};

struct TestArchitecture3 {
    template<typename T, size_t rank>
    using GridStorageType = schnek::LazyArrayGridStorage<T, rank>;

    static const std::string id;

    typedef TestArchitecture PreferredCopySource;
    typedef schnek::generic::TypeList<TestArchitecture, TestArchitecture2> AllowedCopySources;
};

const std::string TestArchitecture::id{"TestArchitecture"};
const std::string TestArchitecture2::id{"TestArchitecture2"};
const std::string TestArchitecture3::id{"TestArchitecture3"};

struct TestFunction {
  double operator()(double x) {
    return x * x;
  }
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

BOOST_AUTO_TEST_CASE( registerFieldFactory )
{
  schnek::computation
    ::MultiArchitectureFieldFactory< schnek::computation::FieldTypeWrapper<int, 2> > factoryInt;
  schnek::computation
    ::MultiArchitectureFieldFactory< schnek::computation::FieldTypeWrapper<double, 2> > factoryDouble;
  schnek::computation::Algorithm<TestArchitecture> algorithm;
  TestFunction testFunction;

  auto regA = algorithm.registerFieldFactory(factoryInt);
  auto regB = algorithm.registerFieldFactory(factoryInt);
  auto regC = algorithm.registerFieldFactory(factoryDouble);
  auto regD = algorithm.registerFieldFactory(factoryDouble);

  BOOST_CHECK_NE(regA.getId(), regB.getId());
  BOOST_CHECK_NE(regA.getId(), regC.getId());
  BOOST_CHECK_NE(regA.getId(), regD.getId());

  auto step = algorithm.stepBuilder<2, TestArchitecture>()
    .input(regA, schnek::generic::size_to_type<2>())
    .input(regB, schnek::generic::size_to_type<2>())
    .output(regC, schnek::generic::size_to_type<2>())
    .output(regD, schnek::generic::size_to_type<2>())
    .build(testFunction);
  
  algorithm.addStep(step);
  auto actions = algorithm.makeActions();
}

BOOST_AUTO_TEST_CASE( architecture )
{
  schnek::computation::Architecture<TestArchitecture3> architecture;
  BOOST_CHECK_EQUAL(architecture.getId(), "TestArchitecture3");
  BOOST_CHECK_EQUAL(architecture.getPreferredCopySource(), "TestArchitecture");
  auto allowedCopySources = architecture.getAllowedCopySources();
  BOOST_CHECK_EQUAL(allowedCopySources.size(), 2);
  BOOST_CHECK_EQUAL(allowedCopySources[0], "TestArchitecture");
  BOOST_CHECK_EQUAL(allowedCopySources[1], "TestArchitecture2");
}

BOOST_AUTO_TEST_CASE( field_store )
{
  typedef schnek::computation::FieldStore<
    schnek::computation::GridTypeWrapper<double, 2>,
    TestArchitecture,
    TestArchitecture2,
    TestArchitecture3
  > Grid2dStorage;

  Grid2dStorage store;
  schnek::Array<int, 2> lo(0, 10);
  schnek::Array<int, 2> hi(100, 150);
  store.fields.push_back(std::make_tuple(
    schnek::Grid<double, 2, schnek::GridNoArgCheck, TestArchitecture::GridStorageType>(lo, hi),
    std::optional<schnek::Grid<double, 2, schnek::GridNoArgCheck, TestArchitecture2::GridStorageType>>(),
    schnek::Grid<double, 2, schnek::GridNoArgCheck, TestArchitecture3::GridStorageType>(lo, hi)
  ));

}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()