The ``Grid`` class defines a type called ``IndexType``. In the previous
example we have seen that this type is used to specify the extent of the
grid. However, it can also be used to index locations in the grid. In
the following example we define a three dimensional grid of size
11x11x11 and fill a single line with the value 1.

::

      Grid<double, 3> grid(Grid<double, 3>::IndexType(0, 0, 0),
          Grid<double, 3>::IndexType(10, 10, 10));

      grid = 0.0;

      Grid<double, 3>::IndexType index(5, 5, 5);

      for (int i = 0; i <= 10; i++)
      {
        index[1] = i;
        grid[index] = 1.0;
      }

In this a rather contrived example ``index`` is of the ``IndexType``
specified by the ``Grid`` class. As you can see you can access
individual elements from the ``IndexType`` by using square brackets as
in the following line.

::

        index[1] = i;

This sets the element at position 1, i.e. the second component, to the
value of ``i``. We can use the square brackets to get elements from the
grid using an index as seen in the following line.

::

        grid[index] = 1.0;

The class behind the ``IndexType`` is the ``Array`` class. ``Array`` is
a fixed length array that acts much like a C-style array.

::

      Array<int, 3> a;

      for (int i = 0; i < 3; ++i)
        a[i] = 2 * i + 1;

This piece of code declares an ``Array`` of size 3 and fills it with
values in a loop. Initialisation can also be done using the constructor
as follows.

::

      Array<int, 3> b(2, 3, 4);

**Note** that Schnek does not yet provide C++11 style Initialisation of
arrays. Of course you can define ``Array``\ s of different types and
dimensionality. For example the following line will create an array of
five strings and initialise it with values.

::

      Array<std::string, 5> s("a","b","c","d","e");

One useful feature of Schnek ``Array``\ s is the ability to use them in
vector style expressions. Loook at the following code.

::

      Array<int, 3> a(4, 5, 6);
      Array<int, 3> b(2, 3, 4);

      Array<int, 3> c;

      c = a + b;

Here we have declared three arrays ``a``, ``b``, and ``c``. ``a`` and
``b`` are initialised with values. In the last line ``c`` is calculated
as the sum of ``a`` and ``b``.

::

      c = a + b;

Schnek uses `expression
templates <http://en.wikipedia.org/wiki/Expression_templates>`__ to
evaluate expressions involving ``Array``\ s. This means that the
evaluation does not create temporary ``Array`` objects and you can
safely use ``Array`` expressions in your code without speed penalty.
``Array`` expressions allow you to add or subtract two arrays and to
multiply or divide an array with a scalar value. Note that the ``Array``
template parameters have to match for arrays that you want to include in
an expression. The code for this example can be found
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_array.cpp>`__.
