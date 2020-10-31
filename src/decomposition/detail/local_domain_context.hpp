/*
 * local_domain_context.hpp
 *
 * Created on: 31 Oct 2020
 * Author: Holger Schmitz (holger@notjustphysics.com)
 */

#ifndef SCHNEK_DECOMPOSITION_DETAIL_LOCAL_DOMAIN_CONTEXT_HPP_
#define SCHNEK_DECOMPOSITION_DETAIL_LOCAL_DOMAIN_CONTEXT_HPP_

#include "../domaindecomposition.hpp"

#include <boost/scoped_ptr.hpp>

#include <list>
#include <vector>

namespace schnek {

namespace detail {

template<int rank, class GridType, template<int> class CheckingPolicy = ArrayNoArgCheck>
class LocalDomainIteratorImpl : public schnek::LocalDomainIterator
{
  public:
    typedef schnek::LocalDomain<rank, CheckingPolicy> LocalDomain;
    typedef boost::shared_ptr<LocalDomain> pLocalDomain;
    typedef typename schnek::LocalDomainContext<rank, CheckingPolicy>::GridFactory<GridType> GridFactory;
  private:
    typedef boost::scoped_ptr<GridType> pGridType;
    GridFactory &gridFactory;
    std::vector<pLocalDomain> domains;
    std::vector<std::list<pGridType>> grids;
    std::list<GridType**> gridRefs;

    int pos;

    void updateGridRefs()
    {
      if (pos>=grids.size())
      {
        return;
      }

      typename std::list<GridType**>::iterator ref = gridRefs.begin();
      typename std::list<pGridType>::iterator gr = grids[pos].begin();
      while (ref != gridRefs.end())
      {
        **gr = ref->get();
        ++gr;
        ++ref;
      }
    }
  public:
    LocalDomainIteratorImpl(GridFactory &gridFactory,
                            const std::vector<pLocalDomain> &domains)
      : gridFactory(gridFactory),
        domains(domains),
        pos(0)
    {
    }

    void reset()
    {
      pos = 0;
      updateGridRefs();
    }

    bool next()
    {
      ++pos;
      updateGridRefs();
    }

    void exchange()
    {

    }

    const LocalDomain &getDomain() const
    {
      return *domains[pos];
    }

    void registerGrid(GridType*& grid)
    {
      gridRefs.push_back(&grid);
      for (int i=0; i<domains.size(); ++i)
      {
        pGridType pgrid(gridFactory.newGrid(*(domains[i])));
        grids[i].push_back(pgrid);
        if (pos == i)
        {
          grid = pgrid.get();
        }
      }
    }

    void registerGrid(std::vector<GridType*>& grids)
    {
      for (int i=0; i<grids.size(); ++i)
      {
        registerGrid(grids[i]);
      }
    }
};

template<int rank, template<int> class CheckingPolicy = ArrayNoArgCheck>
class LocalDomainContextImpl : public schnek::LocalDomainContext
{
  public:
    typedef schnek::LocalDomain<rank, CheckingPolicy> LocalDomain;
    typedef boost::shared_ptr<LocalDomain> pLocalDomain;
  private:
    std::vector<pLocalDomain> domains;

  public:
    template<class GridType>
    LocalDomainIterator<rank, GridType, CheckingPolicy> getGridIterator(GridFactory<GridType> &factory)
    {
      return LocalDomainIteratorImpl<rank, GridType, CheckingPolicy>(factory, domains);
    }
};

} // namespace detail

} // namespace schnek

#endif /* SCHNEK_DECOMPOSITION_DETAIL_LOCAL_DOMAIN_CONTEXT_HPP_ */
