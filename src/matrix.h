#ifndef SCHNEK_MATRIX_H
#define SCHNEK_MATRIX_H

#include "typetools.h"

#include "fixedarray.h"
#include "matrixcheck.h"
#include "mstorage.h"

namespace schnek {

template<class MatrixType, typename TList>
class IndexedMatrix;

template<class IndexType>
struct IndexCast;


/** An elementary matrix class */
template<
  typename T, 
  int rank,
  template<int> class CheckingPolicy = MatrixNoArgCheck,
  template<typename, int> class StoragePolicy = SingleArrayMatrixStorage
>
class Matrix : public StoragePolicy<T,rank>, public CheckingPolicy<rank> {

  public:
  
    typedef FixedArray<int,rank> IndexType;
    
    typedef Matrix<T,rank,CheckingPolicy,StoragePolicy> MatrixType;
  
    /** default constructor creates an empty matrix */
    Matrix();

    /** constructor, which builds Matrix of size size[0] x ... x size[rank-1]
     * 
     *  Example: 
     *  \begin{verbatim}
     *  Matrix<double,2>::IndexType size=(512,512);
     *  Matrix<double,2> m(size);
     *  \end{verbatim}
     * 
     *  The ranges then extend from 0 to size[i]-1
     */
    Matrix(const IndexType &size);

    /** constructor, which builds Matrix with lower indices low[0],...,low[rank-1]
     *  and upper indices high[0],...,high[rank-1]
     * 
     *  Example: 
     *  \begin{verbatim}
     *  Matrix<double,2>::IndexType low(-5,-10);
     *  Matrix<double,2>::IndexType high(15,36);
     *  Matrix<double,2> m(l,h);
     *  \end{verbatim}
     * 
     *  The ranges then extend from low[i] to high[i]
     */
    Matrix(const IndexType &low, const IndexType &high);

    /** copy constructor */
    Matrix(const Matrix<T, rank, CheckingPolicy, StoragePolicy>&);
    
    /** index operator, writing */
    T& operator[](const IndexType& pos); // write
    /** index operator, reading */
    T  operator[](const IndexType& pos) const; // read
    /** index operator, writing */
    T& operator()(int i);
    /** index operator, reading */
    T  operator()(int i) const;
    /** index operator, writing */
    T& operator()(int i, int j);
    /** index operator, reading */
    T  operator()(int i, int j) const;
    /** index operator, writing */
    T& operator()(int i, int j, int k);
    /** index operator, reading */
    T  operator()(int i, int j, int k) const;
    /** index operator, writing */
    T& operator()(int i, int j, int k, int l);
    /** index operator, reading */
    T  operator()(int i, int j, int k, int l) const;
    /** index operator, writing */
    T& operator()(int i, int j, int k, int l, int m);
    /** index operator, reading */
    T  operator()(int i, int j, int k, int l, int m) const;

    template<typename Arg0>
    IndexedMatrix<MatrixType, TYPELIST_1(Arg0) > operator()(
      const Arg0 &i0
    );

    template<typename Arg0, typename Arg1>
    IndexedMatrix<MatrixType, TYPELIST_2(Arg0, Arg1) > operator()(
      const Arg0 &i0, const Arg1 &i1
    );
        
    /** assign another matrix */
    Matrix<T, rank, CheckingPolicy, StoragePolicy>& 
      operator=(const Matrix<T, rank, CheckingPolicy, StoragePolicy>&);

    /** assign a value */
    Matrix<T, rank, CheckingPolicy, StoragePolicy>& 
      operator=(const T &val);
    
    /** Resize to size[0] x ... x size[rank-1]
     * 
     *  Example: 
     *  \begin{verbatim}
     *  Matrix<double,2>::IndexType size=(512,512);
     *  Matrix<double,2> m;
     *  m.resize(size);
     *  \end{verbatim}
     * 
     *  The ranges then extend from 0 to size[i]-1
     */        
    void resize(const IndexType &size);

    /** Resize to lower indices low[0],...,low[rank-1]
     *  and upper indices high[0],...,high[rank-1]
     * 
     *  Example: 
     *  \begin{verbatim}
     *  Matrix<double,2>::IndexType low(-5,-10);
     *  Matrix<double,2>::IndexType high(15,36);
     *  Matrix<double,2> m;
     *  m.resize(l,h);
     *  \end{verbatim}
     * 
     *  The ranges then extend from low[i] to high[i]
     */
    void resize(const IndexType &low, const IndexType &high);
   
    /** Resize to match the size of another matrix */
    void resize(const Matrix<T, rank>& matr);
  private:
    // assumes that the sizes are already set properly
    void copyFromMatrix(const Matrix<T, rank, CheckingPolicy, StoragePolicy>& matr);
};

} // namespace schnek

#include "matrix.t"

#endif
