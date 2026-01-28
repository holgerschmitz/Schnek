/*
 * mock_domain_decomposition.hpp
 *
 * Created on: 28 Jan 2026
 * Author: Holger Schmitz
 */

#ifndef SCHNEK_TESTSUITE_DECOMPOSITION_MOCK_DOMAIN_DECOMPOSITION_HPP_
#define SCHNEK_TESTSUITE_DECOMPOSITION_MOCK_DOMAIN_DECOMPOSITION_HPP_

#include <decomposition/domaindecomposition.hpp>

namespace schnek {
  namespace test {

    /**
     * @brief Mock implementation of DomainDecomposition for testing
     *
     * This mock exposes the protected addLocalRange method so that tests
     * can add local ranges and verify the grid registration and forEach functionality.
     */
    template<size_t rank, template<size_t> class CheckingPolicy = schnek::ArrayNoArgCheck>
    class MockDomainDecomposition : public schnek::DomainDecomposition<rank, CheckingPolicy> {
      public:
        using Base = schnek::DomainDecomposition<rank, CheckingPolicy>;
        using RangeType = typename Base::RangeType;
        using DomainType = typename Base::DomainType;
        using RegistrationVariant = typename Base::RegistrationVariant;

        MockDomainDecomposition() : Base() {}

        void init() override {}
        void balanceLoad() override {}
        int getUniqueId() const override { return 0; }
        bool master() const override { return true; }
        int numProcs() const override { return 1; }

        void exchangeGrid(const schnek::internal::pGridWrapper &wrapper, bool useFieldInfo) override {
          (void)wrapper;
          (void)useFieldInfo;
        }

        void accumulate(const schnek::internal::pGridWrapper &wrapper, bool useFieldInfo) override {
          (void)wrapper;
          (void)useFieldInfo;
        }

        double avgReduce(double value) const override { return value; }
        int avgReduce(int value) const override { return value; }
        long avgReduce(long value) const override { return value; }

        double sumReduce(double value) const override { return value; }
        int sumReduce(int value) const override { return value; }
        long sumReduce(long value) const override { return value; }

        double maxReduce(double value) const override { return value; }
        int maxReduce(int value) const override { return value; }
        long maxReduce(long value) const override { return value; }

        double minReduce(double value) const override { return value; }
        int minReduce(int value) const override { return value; }
        long minReduce(long value) const override { return value; }

        void testAddLocalRange(RangeType range, DomainType domain) {
          this->addLocalRange(range, domain);
        }

        void testAddLocalIterationRange(RangeType range) {
          this->addLocalIterationRange(range);
        }

        template<class GridType>
        schnek::GridRegistration registerField(const schnek::GridFactory<GridType> &factory) {
          return this->registerFieldImpl(factory);
        }

        template<class GridType>
        schnek::GridRegistration registerField(const schnek::GridFactory<GridType> &factory, const RangeType &subRange) {
          return this->registerFieldImpl(factory, subRange);
        }

        template<class GridType>
        typename Base::template ProjectedRegistration<GridType::Rank> registerProjection(
            schnek::GridFactory<GridType> &factory,
            const std::array<size_t, GridType::Rank> &axes
        ) {
          return this->registerFieldProjectionImpl(factory, axes);
        }
    };

  }  // namespace test
}  // namespace schnek

#endif  // SCHNEK_TESTSUITE_DECOMPOSITION_MOCK_DOMAIN_DECOMPOSITION_HPP_
