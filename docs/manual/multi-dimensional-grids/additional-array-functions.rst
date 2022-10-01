In the `previous
tutorial <http://www.notjustphysics.com/schnek/schnek-documentation/multi-dimensional-grids/index-types-and-the-array-class/>`__
we have seen that ``Array``\ s can be used in array expressions. This
means that we can write the following.

::

        Array<double, 3> a(4., 5., 6.);
        Array<double, 3> b(2., 3., 4.);

        Array<double, 3> c;

        c = 2.*(a + b)/3.;

        c -= a;
        c *= 1.5;

You can multiply and divide arrays with numbers and add or subtract two
arrays. Schnek uses expression templates for the evaluation. This means
that the evaluation of an expression does not generate temporary
objects. The evaluation of the expression will be as fast as writing the
loop over all the elements in the vector yourself. Note however, that
type promotion has not yet been implemented. This means that you can not
write the following.

::

        c = 2*(a + b)/3; // compiler error

The difference here is that the number literals are of integer type,
they don't have a trailing decimal point. For now the numbers have to be
of the same type as the value type of the array. Arrays provide a number
of additional functions, useful for everyday calculations. Here are some
examples.

::

        Array<int, 3> a;

        a.fill(3);
        a.at(1) = 5;
        std::cout << "Product: " << a.product() << std::endl;
        std::cout << "Sum: " << a.sum() << std::endl;
        std::cout << "Square: " << a.sqr() << std::endl;

        Array<int,2> b = a.project<2>();

-  The method ``fill`` fills all elements in the array with a single
   value.
-  ``at`` is the accessor method and behaves exactly the same as the
   square bracket index operator.
-  The method ``product`` calculates the product of all the elements.
-  The method ``sum`` calculates the sum of all the elements.
-  The method ``sqr`` calculates the sum of squares of all the elements.
-  To convert a longer array to a shorter array, you can use the
   ``project`` method. Note that you have to explicitly specify the
   number of dimensions of the projected array.

The ``Array`` class also provides two static methods providing easy
construction of the zero vector and an array filled with ones.

::

        Array<int, 3> zero = Array<int, 3>::Zero();
        Array<int, 3> ones = Array<int, 3>::Ones();

The code for these examples can be found as part of the Schnek package
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_array_functions.cpp>`__.
