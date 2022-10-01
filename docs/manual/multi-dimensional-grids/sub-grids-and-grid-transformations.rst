Sub Grids
---------

Let's imagine you have a function that takes a ``Grid`` as an argument
and modifies its values by iterating over all its entries and applying a
transformation. An example is given below.

::

    template<class GridType>
    void squareValues(GridType &grid) {
      Array<int, 2> lo = grid.getLo();
      Array<int, 2> hi = grid.getHi();

      for (int i=lo[0]; i<=hi[0]; ++i)
        for (int j=lo[1]; j<=hi[1]; ++j)
          grid(i,j) = grid(i,j)*grid(i,j);
    }

Note how in this example the function is templated using ``GridType``
and we do not explicitely fix the type of the grid. This is not just
needed for the sake of what comes next. As we have seen in the previous
sections, the behaviour ``Grid`` class can be fine tuned by its template
arguments, such as storage policies and argument checking policies.
Changing the parameters results in a change of the C++ type of the grid.
In order to be flexible, it is often advisable to use template arguments
to create a generic function. Back to the real motivation for this
section. The function above takes the range of indices from the ``grid``
parameter using the ``getLo()`` and ``getHi()`` functions. Now, let's
assume next that you have a ``Grid`` object with values such as the
following.

::

    Array<int,2> lo(10,10);
    Array<int,2> hi(20,20);
    Grid<int, 2> grid(lo,hi);

    grid = 2;

All the values in the grid are now set to 2. Finally imagine that you
want to apply the ``squareValues()`` function to the grid, but not to
all its values. Instead you want to apply it only to the interior,
leaving a border of 3 untouched.

::

    Range<int, 2> range(grid.getLo(), grid.getHi());
    range.grow(-3);

The ``range`` in the code snippet above now includes the region in the
grid that should be modified. Using the ``SubGrid`` class template it is
now possible to construct a grid object that provides a window only to
the values in the range.

::

    SubGrid<Grid<int, 2> > subGrid(range,grid);

    squareValues(subGrid);

The ``SubGrid`` takes two template arguments. The first argument is the
grid type that should be wrapped. The second argument is optional and
provides the checking policy that should be used. This defaults to
``GridNoArgCheck`` but can be any checking policy that can also be
applied to the ``Grid`` class. The result of the example is a grid with
the following values.

::

    2 2 2 2 2 2 2 2 2 2 2 
    2 2 2 2 2 2 2 2 2 2 2 
    2 2 2 2 2 2 2 2 2 2 2 
    2 2 2 4 4 4 4 4 2 2 2 
    2 2 2 4 4 4 4 4 2 2 2 
    2 2 2 4 4 4 4 4 2 2 2 
    2 2 2 4 4 4 4 4 2 2 2 
    2 2 2 4 4 4 4 4 2 2 2 
    2 2 2 2 2 2 2 2 2 2 2 
    2 2 2 2 2 2 2 2 2 2 2 
    2 2 2 2 2 2 2 2 2 2 2 

The code for this example on subgrids can be found
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_grids_subgrid.cpp>`__.

Grid Transformations
--------------------

Schnek also allows you to apply *on-the-fly* transformations to values
in a grid. A transformation is a wrapper around a ``Grid`` object that
calculates modified values on demand. This can be useful if you have a
large grid with data that you don't want to copy. If some function needs
to operate on modified data derived from that grid then transformations
can be helpful. Let's start with such a function. The example below
defines a function that calculates the square of the discrete Laplace
operator of an input grid and stores it in an output grid.

::

    template<class GridTypeIn, class GridTypeOut>
    void laplace_sq(GridTypeIn &gridIn, GridTypeOut &gridOut) {
      Array<int,2> lo = gridIn.getLo();
      Array<int,2> hi = gridIn.getHi();

      for (int i=lo[0]+1; i<hi[0]; ++i)
        for (int j=lo[1]+1; j<hi[1]; ++j) {
             double laplace = gridIn(i-1,j) + gridIn(i+1,j)
                            + gridIn(i,j-1) + gridIn(i,j+1)
                            - 4*gridIn(i,j);
             gridOut(i,j) = laplace*laplace;
        }
    }

This function uses a standard five point stencil to apply the discrete
Laplace operator to the ``gridIn`` parameter. It then stores the squared
result in the ``gridOut`` parameter. Let's construct an input grid and
fill it with some values.

::

    const int N = 200;
    Array<int,2> lo(-N,-N);
    Array<int,2> hi( N, N);
    Grid<double, 2> grid(lo, hi);

    for (int i=lo[0]; i<=hi[0]; ++i)
      for (int j=lo[1]; j<=hi[1]; ++j)
        grid(i,j) = sqrt(i*i +j*j);

[caption id="attachment\_444" align="alignright" width="150"]\ |A grid
storing the distance from the center| A grid storing the distance from
the centre[/caption] The values in this grid contain the distance to the
``(0,0)`` coordinate. We could apply the ``laplace_sq()`` function to
this grid straight away, but this would not be very interesting.
Instead, we want to first apply a transformation to it. Transformations
are encapsulated in function object, such as the following.

::

    class SuperGauss {
      private:
        double r;
      public:
        SuperGauss(double r_ = 1.0) : r(r_) {}
        double operator()(double x) const {
          return exp(-std::pow(x/r, 4));
        }
    };

The class ``SuperGauss`` defines a function operator that takes a
``double x`` as parameter and returns a transformed ``double``. In our
case the transformation is given by a super-Gaussian function of order
4. We are now ready to construct a transformed view onto the data stored
in the ``grid`` object.

::

    GridTransform<Grid<double, 2>, SuperGauss> gridT(grid);

[caption id="attachment\_446" align="alignright" width="150"]\ |A super
Gaussian calculated from the distance to the centre.| A super Gaussian
calculated from the distance to the centre.[/caption] ``gridT`` is a
``GridTransform`` object that returns the values of the underlying
``grid``, after they have passed through the ``SuperGauss``
transformation. The transformation has been created using its default
constructor. If you wish to replace this default constructed
transformation you can replace it using the ``setTransformation()``
method.

::

    gridT.setTransformation(SuperGauss(100.0));

Note that ``gridT`` does not store any transformed values. The function
operator is evaluated every time a grid value is accessed. If the
transformation is a lengthy calculation this means that there is a
penalty for re-calculating elements in the grid multiple times. The
transformed grid can now be fed into the ``laplace_sq()`` function. This
will result in the calculation of the squared Laplacian of the
super-Gaussian, as seen below. [caption id="attachment\_445"
align="aligncenter" width="401"]\ |The square of the Laplacian of the
super Gaussian.| The square of the Laplacian of the super
Gaussian.[/caption] The code for this example on grid transformations
can be found
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_grids_transform.cpp>`__.

.. |A grid storing the distance from the center| image:: http://www.notjustphysics.com/wp-content/uploads/2017/01/example_grids_transform_gridIn-150x150.png
   :class: wp-image-444 size-thumbnail
   :width: 150px
   :height: 150px
   :target: http://www.notjustphysics.com/wp-content/uploads/2017/01/example_grids_transform_gridIn.png
.. |A super Gaussian calculated from the distance to the centre.| image:: http://www.notjustphysics.com/wp-content/uploads/2017/01/example_grids_transform_gridT-150x150.png
   :class: wp-image-446 size-thumbnail
   :width: 150px
   :height: 150px
   :target: http://www.notjustphysics.com/wp-content/uploads/2017/01/example_grids_transform_gridT.png
.. |The square of the Laplacian of the super Gaussian.| image:: http://www.notjustphysics.com/wp-content/uploads/2017/01/example_grids_transform_gridOut.png
   :class: wp-image-445 size-full
   :width: 401px
   :height: 401px
   :target: http://www.notjustphysics.com/wp-content/uploads/2017/01/example_grids_transform_gridOut.png
