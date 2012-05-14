/*
 * field.t
 *
 *  Created on: 9 May 2012
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */

using namespace schnek

template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
static IndexType Field<T, rank, CheckingPolicy, StoragePolicy>
  ::calcGridSize(const IndexType &size, const Stagger &stagger)
{
  IndexType gridSize(size);
  for (int i=0; i<rank; ++i)
    if (!stagger[i]) ++gridSize[i];
  return gridSize;
}

template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
Field<T, rank, CheckingPolicy, StoragePolicy>::Field()
{}

template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
Field<T, rank, CheckingPolicy, StoragePolicy>
  ::Field(const IndexType &size_, const FieldRange &range_, const Stagger &stagger_)
  : Grid(calcGridSize(size_,stagger)),
    range(range_),
    stagger(stagger_),
    size(size_)
{}

template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
inline void Field<T, rank, CheckingPolicy, StoragePolicy>::positionToIndex(int dim, double pos, int &index, double &offset)
{
    double xnorm = pos*size[dim]/(range.getMax()[dim] - range.getMin()[dim])
        - 0.5*int(stagger[dim]);
    index = int(xnorm);
    offset = xnorm - index;
}

template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
inline int Field<T, rank, CheckingPolicy, StoragePolicy>::positionToIndex(int dim, double pos)
{
    return int(pos*size[dim]/(range.getMax()[dim] - range.getMin()[dim])
        - 0.5*int(stagger[dim]));
}

template<
  typename T,
  int rank,
  template<int> class CheckingPolicy,
  template<typename, int> class StoragePolicy
>
inline double Field<T, rank, CheckingPolicy, StoragePolicy>::indexToPosition(int dim, int index)
{
    return (range.getMax()[dim] - range.getMin()[dim])
        * (index+0.5*int(stagger[dim]))/size[dim]
}
