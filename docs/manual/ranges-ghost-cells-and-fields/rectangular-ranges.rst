Ranges represent rectangular domains of arbitrary dimensions. In
principal a range is simply a pair of coordinates and, as such, can be
constructed with two coordinates. The ``Range`` class takes two template
arguments specifying the data type stored in the range and the
dimensionality of the range.

::

      Range<int, 2> range;

The code above creates a two dimensional range of integers. The
``Range`` class defines the type ``LimitType`` that is used for
specifying the lower and upper bounds of the range. An additional third
template argument allows for control over argument checking policies. In
order to create a two dimensional range from, let's say, (1,2) to (4,5)
we can write the following.

::

      Range<int, 2>::LimitType low(1, 2);
      Range<int, 2>::LimitType high(4, 5);

      Range<int, 2> range(low, high);

The upper bound of the range is inclusive. This means that the range
contains all the points lying in the rectangle spanning from the point
(1,2) to, and including, the point (4,5). Note that no check is carried
out to ensure that the upper bound is larger than the lower bound. The
methods ``getLo()`` and ``getHi()`` return the lower and upper bounds of
the range. The following code assigns the lower bound to low and the
upper bound to high.

::

      low  = range.getLo();
      high = range.getHi();

The ``getLo()`` and ``getHi()`` also allow write access to the bounds of
the range. So we can modify the range as follows.

::

      range.getLo() = Range<int, 2>::LimitType(5,2);
      range.getHi() = Range<int, 2>::LimitType(9,7);

This will change the range to the rectangle (5,2) to (9,7). The
``Range`` class supplies a utility method that allows you to grow or
shrink the range in all directions by the same amount. Given the range
above the following call will shrink the range by 1, resulting in a
rectangular range from (6,3) to (8,6).

::

      range.grow(-1);

For ranges containing integer types we can use the iterator supplied by
the ``Range`` class to iterate over all coordinates that lie within the
range.

::

      for (Range<int, 2>::iterator it=range.begin(); it!=range.end(); ++it)
      {
        const Range<int, 2>::LimitType &pos = *it;
        std::cout << pos[0] << " " << pos[1] << std::endl;
      }

The code above iterates over the rectangle and prints out all the
coordinates. The output of the code is as follows.

::

    6 3
    6 4
    6 5
    6 6
    7 3
    7 4
    7 5
    7 6
    8 3
    8 4
    8 5
    8 6

The code for this tutorial can be found
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_range.cpp>`__.
