/*
 * test_hdfdiagnostic.cpp
 *
 * Created on: 28 Jan 2026
 * Author: Holger Schmitz
 */

#include "../utility.hpp"

#include <diagnostic/hdfdiagnostic.hpp>
#include <grid/grid.hpp>

#include "../decomposition/mock_domain_decomposition.hpp"

#include <boost/test/unit_test.hpp>

#include <cstdio>
#include <string>

#ifdef SCHNEK_HAVE_HDF5

namespace {

  using schnek::test::MockDomainDecomposition;

  class RegistrationDiagnosticTest : public schnek::HDFGridRegistrationDiagnostic<
                                         schnek::Grid<double, 2>,
                                         MockDomainDecomposition<2>,
                                         schnek::IntervalDiagnostic> {
    public:
      using Base = schnek::HDFGridRegistrationDiagnostic<
          schnek::Grid<double, 2>,
          MockDomainDecomposition<2>,
          schnek::IntervalDiagnostic>;
      using IndexType = typename schnek::Grid<double, 2>::IndexType;

      RegistrationDiagnosticTest(MockDomainDecomposition<2> &decompositionIn)
          : decomposition(&decompositionIn) {}

      void setRegistrationName(const std::string &name) { registrationName = name; }
      void setGlobalRange(const IndexType &lo, const IndexType &hi) {
        globalMin = lo;
        globalMax = hi;
      }

      using Base::close;
      using Base::init;
      using Base::open;
      using Base::write;

    protected:
      IndexType getGlobalMin() override { return globalMin; }
      IndexType getGlobalMax() override { return globalMax; }

      MockDomainDecomposition<2> &getDecomposition() override {
        return *decomposition;
      }

    private:
      MockDomainDecomposition<2> *decomposition;
      IndexType globalMin;
      IndexType globalMax;
  };

  class GridDiagnosticTest : public schnek::HDFGridDiagnostic<schnek::Grid<double, 2>, schnek::IntervalDiagnostic> {
    public:
      using Base = schnek::HDFGridDiagnostic<schnek::Grid<double, 2>, schnek::IntervalDiagnostic>;
      using IndexType = typename schnek::Grid<double, 2>::IndexType;

      void setGrid(const schnek::Grid<double, 2> &gridIn) { grid = gridIn; }
      void setGlobalRange(const IndexType &lo, const IndexType &hi) {
        globalMin = lo;
        globalMax = hi;
      }

      using Base::close;
      using Base::open;
      using Base::write;

      void runInit() { init(); }

    protected:
      bool isDerived() override { return true; }

      void init() override {
        this->field = grid;
        schnek::CopyToContainer<schnek::Grid<double, 2>>::copy(this->field, this->container);
        this->container.global_min = getGlobalMin();
        this->container.global_max = getGlobalMax();
      }

      IndexType getGlobalMin() override { return globalMin; }
      IndexType getGlobalMax() override { return globalMax; }

    private:
      schnek::Grid<double, 2> grid;
      IndexType globalMin;
      IndexType globalMax;
  };

}  // namespace

BOOST_AUTO_TEST_SUITE(diagnostic)

BOOST_AUTO_TEST_SUITE(hdf)

BOOST_AUTO_TEST_CASE(grid_registration_writes_and_reads)
{
  using GridType = schnek::Grid<double, 2>;
  using RangeType = schnek::Range<ptrdiff_t, 2>;
  using DomainType = schnek::Range<double, 2>;

  GridType::IndexType lo(0, 0);
  GridType::IndexType hi(2, 3);
  RangeType globalRange(lo, hi);
  DomainType domain(schnek::Array<double, 2>(0.0, 0.0), schnek::Array<double, 2>(1.0, 1.0));

  MockDomainDecomposition<2> decomposition;
  decomposition.setGlobalRange(globalRange);
  decomposition.setGlobalDomain(domain);
  decomposition.testAddLocalRange(globalRange, domain);

  schnek::GridFactory<GridType> factory;
  schnek::GridRegistration registration = decomposition.registerField(factory);

  auto context = decomposition.getGridContext({registration});
  context.forEach([&](const RangeType &, GridType &grid) {
    for (ptrdiff_t i = lo[0]; i <= hi[0]; ++i) {
      for (ptrdiff_t j = lo[1]; j <= hi[1]; ++j) {
        grid(i, j) = static_cast<double>(i * 10 + j);
      }
    }
  });

  RegistrationDiagnosticTest diagnostic(decomposition);
  diagnostic.setRegistrationName("grid");
  diagnostic.setGlobalRange(lo, hi);
  diagnostic.addData("grid", registration);
  diagnostic.init();

  const std::string filename = "test_hdf_registration_diag.h5";
  diagnostic.open(filename);
  diagnostic.write();
  diagnostic.close();

  GridType output(lo, hi);
  schnek::GridContainer<GridType> container;
  container.grid = output;
  container.global_min = lo;
  container.global_max = hi;
  container.local_min = lo;
  container.local_max = hi;

  schnek::HdfIStream input(filename.c_str());
  input.setBlockName("data");
  input.readGrid(container);
  input.close();

  for (ptrdiff_t i = lo[0]; i <= hi[0]; ++i) {
    for (ptrdiff_t j = lo[1]; j <= hi[1]; ++j) {
      BOOST_CHECK_EQUAL(container.grid(i, j), static_cast<double>(i * 10 + j));
    }
  }

  std::remove(filename.c_str());
}

BOOST_AUTO_TEST_CASE(grid_diagnostic_writes_and_reads)
{
  using GridType = schnek::Grid<double, 2>;

  GridType::IndexType lo(0, 0);
  GridType::IndexType hi(1, 2);

  GridType grid(lo, hi);
  for (ptrdiff_t i = lo[0]; i <= hi[0]; ++i) {
    for (ptrdiff_t j = lo[1]; j <= hi[1]; ++j) {
      grid(i, j) = static_cast<double>(i + j * 0.5);
    }
  }

  GridDiagnosticTest diagnostic;
  diagnostic.setGrid(grid);
  diagnostic.setGlobalRange(lo, hi);
  diagnostic.runInit();

  const std::string filename = "test_hdf_grid_diag.h5";
  diagnostic.open(filename);
  diagnostic.write();
  diagnostic.close();

  GridType output(lo, hi);
  schnek::GridContainer<GridType> container;
  container.grid = output;
  container.global_min = lo;
  container.global_max = hi;
  container.local_min = lo;
  container.local_max = hi;

  schnek::HdfIStream input(filename.c_str());
  input.setBlockName("data");
  input.readGrid(container);
  input.close();

  for (ptrdiff_t i = lo[0]; i <= hi[0]; ++i) {
    for (ptrdiff_t j = lo[1]; j <= hi[1]; ++j) {
      BOOST_CHECK_EQUAL(container.grid(i, j), static_cast<double>(i + j * 0.5));
    }
  }

  std::remove(filename.c_str());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

#endif  // SCHNEK_HAVE_HDF5
