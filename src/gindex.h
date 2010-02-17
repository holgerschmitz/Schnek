#ifndef SCHNEK_GINDEX_H
#define SCHNEK_GINDEX_H


namespace schnek {

struct GIndex {
  int value;
  int low;
  int high;
};


template<class GridType, typename TList, int dim>
class IndexedGridImpl;

template<class GridType, int dim>
class IndexedGridImpl<GridType, schnek::NullType, dim>
{
  public:    
    GridType *grid;
};

template<class GridType, typename TailList, int dim>
class IndexedGridImpl<GridType, schnek::Typelist<int, TailList >, dim >
  : public IndexedGridImpl<GridType, TailList, dim+1>
{
  public:
    typedef typename schnek::TypeAtNonStrict<TailList,0,schnek::NullType>::Result Arg0;
    typedef typename schnek::TypeAtNonStrict<TailList,1,schnek::NullType>::Result Arg1;

    typedef IndexedGridImpl<GridType, TailList, dim+1> BaseType;
    typedef int IndexType;
  
    int index;

    IndexedGridImpl(int index_) : index(index_) {}
    IndexedGridImpl(int index_, const Arg0 &i1) : index(index_), BaseType(i1) {}
    IndexedGridImpl(int index_, const Arg0 &i1, const Arg1 &i2) : index(index_), BaseType(i1,i2) {}
};

template<class GridType, typename TailList, int dim>
class IndexedGridImpl<GridType, schnek::Typelist<GIndex, TailList >, dim >
  : public IndexedGridImpl<GridType, TailList, dim+1>
{
  public:
    typedef typename schnek::TypeAtNonStrict<TailList,0,schnek::NullType>::Result Arg0;
    typedef typename schnek::TypeAtNonStrict<TailList,1,schnek::NullType>::Result Arg1;

    typedef IndexedGridImpl<GridType, TailList, dim+1> BaseType;
    typedef GIndex IndexType;

    const GIndex &index;
    
    IndexedGridImpl(const GIndex &index_) : index(index_) {}  
    IndexedGridImpl(const GIndex &index_, const Arg0 &i1) : index(index_), BaseType(i1) {}
    IndexedGridImpl(const GIndex &index_, const Arg0 &i1, const Arg1 &i2) : index(index_), BaseType(i1,i2) {}
};

template<class GridType, typename TList>
class IndexedGrid : IndexedGridImpl<GridType, TList, 0>
{
  public:
    typedef IndexedGridImpl<GridType, TList, 0> BaseType;

    typedef typename schnek::TypeAtNonStrict<TList,0,schnek::NullType>::Result Arg0;
    typedef typename schnek::TypeAtNonStrict<TList,1,schnek::NullType>::Result Arg1;

    template<int dim, class IndMatrType = IndexedGrid<GridType,TList> >
    struct getIndex;
  
    template<int dim, class AGridType, class ATList>
    struct getIndex<dim,IndexedGrid<AGridType,ATList> >
      : public getIndex<dim,typename IndexedGrid<AGridType,ATList>::BaseType> { };

    template<int dim, class AGridType, class ATList>
    struct getIndex<dim, IndexedGridImpl<AGridType,ATList, dim> >
    { 
      typedef typename schnek::TypeAtNonStrict<TList,dim,schnek::NullType>::Result IndexType;
      typedef IndexedGridImpl<AGridType,ATList, dim> ResultType;

      static IndexType &get(IndexedGrid<GridType,TList> &IndMatr) {
        return (static_cast< IndexedGridImpl<AGridType,ATList, dim>& >(IndMatr)).getIndex();
      }
    };

    template<int dim, class AGridType, class ATList, int dim2>
    struct getIndex<dim, IndexedGridImpl<AGridType,ATList, dim2> >
      : public getIndex<dim,typename IndexedGridImpl<AGridType,ATList, dim2>::BaseType> { };
    
    IndexedGrid(const Arg0 &ind0) 
      : BaseType(ind0)
    {}

    IndexedGrid(const Arg0 &ind0, const Arg1 &ind1
    ) 
      : BaseType(ind0,ind1)
    {}
};

} // namespace schnek

#include "gindex.t"

#endif
