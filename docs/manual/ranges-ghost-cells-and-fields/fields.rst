The ``Grid`` class in Schnek provides a basic method of storing data on
a numerical grid. In many simulations of physical systems, the grid will
represent a domain in space and the values stored in the grid represent
samples or cell averages of physical quantities. This means that each
value in the grid will be associated with a coordinate in the physical
domain. For a large class of simulations the grid points will be evenly
spaced in the physical domain. For these regular grids the coordinates
of each grid point can be calculated using the grid size, the extent of
the physical domain and some optional *stagger* of the grid. The stagger
specifies for each coordinate dimension whether the value stored in the
grid lives in the cell center or on the cell boundary. In the latter
case, the coordinate of the grid point is shifted by half a grid cell in
the positive direction. Schnek provides the ``Field`` class to make it
easier to deal with regular grids on physical domains. The ``Field``
class extends the ``Grid`` class with some extra functionality. Creating
a ``Field`` is similar to creating a ``Grid`` except that the
constructor takes a few more arguments.

::

    Field<double, 3>::IndexType lo(0,0,0), hi(9,9,9);
    Field<double, 3>::RangeLimit physLo(-0.5, -0.5, -0.5), physHi(0.5, 0.5, 0.5);
    Field<double, 3>::RangeType physRange(physLo, physHi);
    Field<double, 3>::Stagger stagger(false, true, false);

    Field<double, 3> field(lo, hi, physRange, stagger, 2);

The ``lo`` and ``hi`` parameters specify the lower and upper indices of
the grid's inner domain. This is thought to be the region which
corresponds to the physical domain. The physical range is specified by
the second argument, ``physRange``. This is a `Rectangular
Range <rectangular-ranges.html>`__
of ``double`` type that stores the minimum and maximum of the physical
coordinates represented by the field. In the example above, the physical
coordinate of the grid point at index ``lo = (0,0,0)`` is
``(-0.5, -0.5, -0.5)``, whereas the physical coordinate of the grid
point at index ``hi = (10,10,10)`` is ``(0.5, 0.5, 0.5)``. The third
argument specifies the staggering of the grid in each direction. The
``stagger`` array is an array of boolean. Each boolean indicates whether
the grid is staggered in the corresponding direction. In the example the
grid is staggered in the y-direction but not in the x or z-direction.
The last argument allows to set the number of ghost cells of the field.
Ghost cells are often needed in order to implement boundary conditions.
The are also needed in order to implement numerical schemes on
distributed systems. In the example, the field has two ghost cells. This
means that the grid is extended by two cells in each direction. The
following code illustrates the behaviour of the ghost cells.

::

    lo = field.getLo();
    hi = field.getHi();

    std::cout << "Outer Lo Index (" << lo[0] << ", " << lo[1] << ", " << lo[2] << ")" << std::endl;
    std::cout << "Outer Hi Index (" << hi[0] << ", " << hi[1] << ", " << hi[2] << ")" << std::endl;

This piece of code will write the following output.

::

    Outer Lo Index (-2, -2, -2)
    Outer Hi Index (11, 11, 11)

Here the lower and upper index bound of the grid has been shifted by two
cells with regard to the bounds passed to the constructor. In order to
obtain the bounds of the inner computational domain, excluding the ghost
cells, two new methods have been added to the ``Field`` class,
``getInnerLo()`` and ``getInnerHi()``.

::

    lo = field.getInnerLo();
    hi = field.getInnerHi();

    std::cout << "Inner Lo Index (" << lo[0] << ", " << lo[1] << ", " << lo[2] << ")" << std::endl;
    std::cout << "Inner Hi Index (" << hi[0] << ", " << hi[1] << ", " << hi[2] << ")" << std::endl;

The code snippet above will produce the following output.

::

    Inner Lo Index (0, 0, 0)
    Inner Hi Index (9, 9, 9)

Here the lower and upper index bounds of the interior region are
returned. These are the same as the ``lo`` and ``hi`` parameters passed
to the constructor. In order to convert between grid indices and
physical coordinates the ``Field`` class provides the methods
``indexToPosition()`` and ``positionToIndex()``. ``indexToPosition()``
takes two arguments, the direction and a grid index. It will return the
physical coordinate of the grid cell, taking any grid stagger into
account. The following code uses ``indexToPosition()`` to find the
coordinate of the 5th grid cell in the x, y, and z directions.

::

    std::cout << "x_5 = " << field.indexToPosition(0, 5) << std::endl;
    std::cout << "y_5 = " << field.indexToPosition(1, 5) << std::endl;
    std::cout << "z_5 = " << field.indexToPosition(2, 5) << std::endl;

The output produced by these lines is

::

    x_5 = 0
    y_5 = 0.05
    z_5 = 0

Although all three indices are the same, the result of
``indexToPosition()`` is different for the y coordinate. The reason for
this difference is that the grid has been set up to be staggered in the
y direction but not in the x or z direction. The inverse operation is
also possible. To obtain the grid index corresponding to a physical
position one can use ``positionToIndex()``.

::

    std::cout << "Index at x=0.2: " << field.positionToIndex(0, 0.2) << std::endl;
    std::cout << "Index at y=0.2: " << field.positionToIndex(1, 0.2) << std::endl;
    std::cout << "Index at z=0.2: " << field.positionToIndex(2, 0.2) << std::endl;

The ``positionToIndex()`` method returns the integer part of the grid
position rescaled to the grid indices. Any non-integer part is always
rounded down to the next lower integer. The result of the code above is
the following output.

::

    Index at x=0.2: 7
    Index at y=0.2: 6
    Index at z=0.2: 7

Again the difference between the y-index and the x and z-indices is due
to the different staggering on the grid in the different directions. In
many cases it is important to calculate both, an integer grid position
and a floating point offset, relative to the grid coordinate. This is
frequently needed when interpolating values on a grid to arbitrary
positions between the grid cells. For this purpose, there is an
overloaded method of ``positionToIndex()`` that does not return a value
nut instead takes two additional references as arguments. This is
illustrated in the following example.

::

    int index;
    double offset;

    field.positionToIndex(0, 0.2, index, offset);
    std::cout << "Index (offset) at x=0.2: " << index << " (" << offset << ")" << std::endl;
    field.positionToIndex(1, 0.2, index, offset);
    std::cout << "Index (offset) at y=0.2: " << index << " (" << offset << ")" << std::endl;
    field.positionToIndex(2, 0.2, index, offset);
    std::cout << "Index (offset) at z=0.2: " << index << " (" << offset << ")" << std::endl;

Instead of just creating a single integer for each position, this
example will return the following pairs of grid index and grid offset.

::

    Index (offset) at x=0.2: 7 (0)
    Index (offset) at y=0.2: 6 (0.5)
    Index (offset) at z=0.2: 7 (0)

The code for this tutorial can be found
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_fields_basics.cpp>`__.
