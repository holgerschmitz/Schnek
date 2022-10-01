This section will cover the basic usage of multi-dimensional Grids. In
order to use the ``Grid`` class you need to include the
``schnek/grid.hpp`` header. The ``Grid`` class needs to template
arguments. The first argument specifies the type and the second argument
specifies the rank of the grid. Let's look at the following simple
example.

::

    #include <schnek/grid.hpp>

    using namespace schnek;

    int main()
    {
      Grid<double,2> grid;
    }

This will create a two-dimensional grid containing ``double`` elements.

::

    #include <schnek/grid.hpp>

    using namespace schnek;

    int main()
    {
      Grid<double,2> grid(Grid<double,2>::IndexType(5,5));

      grid = 1.0;

      for (int j=0; j<5; ++j)
         grid(j,j) = sqrt(2*j);
    }

Let's go through the example line by line.

::

    Grid<double,2> grid(Grid<double,2>::IndexType(5,5));

This line will create a two dimensional grid of size 5 by 5. To
initialise the size of a ``Grid`` you need to pass a two dimensional
index array to the constructor. The type of the index array is given by
the ``IndexType`` type definition inside the ``Grid`` class. The line

::

      grid = 1.0;

fills all the entries in the ``Grid`` with the value 1. To access
individual entries, you can use the overloaded function operator.

::

      for (int j=0; j<5; ++j)
         grid(j,j) = sqrt(2*j);

These lines will iterate over the index ``j`` and fill the diagonal of
the ``Grid`` with a value given by ``sqrt(2*j)`` After the loop has
finished the grid will contain the following values.

::

    0       1       1       1       1 
    1       1.41421 1       1       1 
    1       1       2       1       1 
    1       1       1       2.44949 1 
    1       1       1       1       2.82843 

**Tip:** It is often useful to use ``typedef``\ s to keep your code
short and readable. The declaration an initialisation in the example
above can be replaced by.

::

      typedef schnek::Grid<double, 2> MyGrid;
      typedef MyGrid::IndexType MyIndex;

      MyGrid grid(MyIndex(5, 5));

While this introduces more code initially, it will increase readability
in the long run.   The code for this tutorial can be
found \ `here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_grids_basics.cpp>`__ under
the heading\ * Example 1*.
