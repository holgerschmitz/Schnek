By default the ``Grid`` class will use a one-dimensional array
internally to store the grid data. The data layout of this array
conforms to the standard C layout of multi-dimensional arrays. This
means that the last index is the index that changes most rapidly. In
other words, the element with a last index of ``k`` and the element with
a last index of ``k+1`` are adjacent in memory. In contrast, an
increment of the first index will correspond to the largest jump in
memory addresses. In most cases this is exactly what you want. In some
cases, however, you would like to modify this behaviour. Remember that
the ``getRawData()`` member function of the ``Grid`` class gives you
access to the underlying data. For example, it may be the case that you
wish to interface with a FORTRAN library. In this case you need to can
pass a pointer to a FORTRAN routine and FORTRAN will accept this pointer
as an array. In FORTRAN, arrays are ordered the other way around. This
means that the first index in a multi-dimensional array is the index
that changes most rapidly, while the last index corresponds to the
largest jumps in memory addresses. Schnek gives you the possibility to
change the standard C layout of the internal array to a FORTRAN layout
using the fourth template parameter of the ``Grid`` class. By default
this template argument is ``SingleArrayGridStorage``. Changing it to
``SingleArrayGridStorageFortran`` will result in a ``Grid`` that uses
the FORTRAN layout internally.

::

    Grid<double, 3, GridNoArgCheck, SingleArrayGridStorageFortran> grid;

The line above will create a rank 3 grid of double values, using no
argument checking and using FORTRAN memory layout. In addition to the
FORTRAN layout Schnek also provides a storage policy that uses lazy
allocation to allocate the memory. This is particularly useful if you
have use a ``Grid`` as a buffer with a size that changes frequently. By
default, a call the ``Grid::resize()`` will de-allocate the memory for
the internal array of the old size and then re-allocate the memory for
the internal array of the new size. However, frequent de-allocations and
re-allocations can be time consuming.

::

    Grid<double, 3, GridNoArgCheck, LazyArrayGridStorage> lazyGrid;

The line above uses the ``LazyArrayGridStorage`` policy to create a grid
with lazy storage. Here calls to ``resize()`` will allocate an array
that is slightly larger than requested. If on a subsequent call to
``resize()`` less memory is needed, the internal memory is not
de-allocated straight away but re-used. The extra memory is kept in case
a future call to ``resize()`` will request a larger chunk of memory.
Only after a large number of ``resize()`` calls with low memory usage
will the extra memory be thrown away. For grids that frequently resize
this results in a compromise between memory usage and time used for
allocations and de-allocations.
