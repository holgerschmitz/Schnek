#ifndef SCHNEK_MINDEX_H
#define SCHNEK_MINDEX_H


namespace schnek {

struct MIndex {
  int value;
  int low;
  int high;
};


template<class MatrixType, typename TList, int dim>
class IndexedMatrixImpl;

template<class MatrixType, int dim>
class IndexedMatrixImpl<MatrixType, rigger::NullType, dim>
{
  public:    
    MatrixType *matrix;
};

template<class MatrixType, typename TailList, int dim>
class IndexedMatrixImpl<MatrixType, rigger::Typelist<int, TailList >, dim >
  : public IndexedMatrixImpl<MatrixType, TailList, dim+1>
{
  public:
    typedef typename rigger::TypeAtNonStrict<TailList,0,rigger::NullType>::Result Arg0;
    typedef typename rigger::TypeAtNonStrict<TailList,1,rigger::NullType>::Result Arg1;

    typedef IndexedMatrixImpl<MatrixType, TailList, dim+1> BaseType;
    typedef int IndexType;
  
    int index;

    IndexedMatrixImpl(int index_) : index(index_) {}
    IndexedMatrixImpl(int index_, const Arg0 &i1) : index(index_), BaseType(i1) {}
    IndexedMatrixImpl(int index_, const Arg0 &i1, const Arg1 &i2) : index(index_), BaseType(i1,i2) {}
};

template<class MatrixType, typename TailList, int dim>
class IndexedMatrixImpl<MatrixType, rigger::Typelist<MIndex, TailList >, dim >
  : public IndexedMatrixImpl<MatrixType, TailList, dim+1>
{
  public:
    typedef typename rigger::TypeAtNonStrict<TailList,0,rigger::NullType>::Result Arg0;
    typedef typename rigger::TypeAtNonStrict<TailList,1,rigger::NullType>::Result Arg1;

    typedef IndexedMatrixImpl<MatrixType, TailList, dim+1> BaseType;
    typedef MIndex IndexType;

    const MIndex &index;
    
    IndexedMatrixImpl(const MIndex &index_) : index(index_) {}  
    IndexedMatrixImpl(const MIndex &index_, const Arg0 &i1) : index(index_), BaseType(i1) {}
    IndexedMatrixImpl(const MIndex &index_, const Arg0 &i1, const Arg1 &i2) : index(index_), BaseType(i1,i2) {}
};

template<class MatrixType, typename TList>
class IndexedMatrix : IndexedMatrixImpl<MatrixType, TList, 0>
{
  public:
    typedef IndexedMatrixImpl<MatrixType, TList, 0> BaseType;

    typedef typename rigger::TypeAtNonStrict<TList,0,rigger::NullType>::Result Arg0;
    typedef typename rigger::TypeAtNonStrict<TList,1,rigger::NullType>::Result Arg1;

    template<int dim, class IndMatrType = IndexedMatrix<MatrixType,TList> >
    struct getIndex;
  
    template<int dim, class AMatrixType, class ATList>
    struct getIndex<dim,IndexedMatrix<AMatrixType,ATList> >
      : public getIndex<dim,typename IndexedMatrix<AMatrixType,ATList>::BaseType> { };

    template<int dim, class AMatrixType, class ATList>
    struct getIndex<dim, IndexedMatrixImpl<AMatrixType,ATList, dim> >
    { 
      typedef typename rigger::TypeAtNonStrict<TList,dim,rigger::NullType>::Result IndexType;
      typedef IndexedMatrixImpl<AMatrixType,ATList, dim> ResultType;

      static IndexType &get(IndexedMatrix<MatrixType,TList> &IndMatr) {
        return (static_cast< IndexedMatrixImpl<AMatrixType,ATList, dim>& >(IndMatr)).getIndex();
      }
    };

    template<int dim, class AMatrixType, class ATList, int dim2>
    struct getIndex<dim, IndexedMatrixImpl<AMatrixType,ATList, dim2> >
      : public getIndex<dim,typename IndexedMatrixImpl<AMatrixType,ATList, dim2>::BaseType> { };
    
    IndexedMatrix(const Arg0 &ind0) 
      : BaseType(ind0)
    {}

    IndexedMatrix(const Arg0 &ind0, const Arg1 &ind1
    ) 
      : BaseType(ind0,ind1)
    {}
};

} // namespace schnek

#include "mindex.t"

#endif
