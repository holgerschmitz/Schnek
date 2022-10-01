By default, when accessing elements in a ``Grid``, Schnek does not check
if the index falls within the valid range of the grid. This behaviour is
much like the standard behaviour of C and C++. If you request an element
outside the range of the grid the code will still attempt to access some
memory address. Whether that memory address falls within the memory
allocated for the grid or not depends circumstances and you might end up
with non-sensical data or, if you are lucky, a segmentation fault.

::

    Grid<double, 3> grid(Array<int, 3>(3,3,3));

    // anything could happen here
    grid(-1,5,10) = 1.0;

The code snippet above shows an example of accessing a grid out of
range. The result is undefined. You could end up crashing the code or
you could unknowingly overwrite a memory address that belongs to some
other variable. This behaviour is usually the best for production code
that is known to be stable because no checking means that the code runs
faster. On the other hand, during the process of developing a simulation
code, you might wish for some notification when you are exceeding the
valid index range. Schnek allows you to choose between different
agrument checking policies by specifying the third template parameter of
the ``Grid`` class. By default this template argument is
``GridNoArgCheck``. The other checking policy that has been implemented
in Schnek is the ``GridAssertCheck`` class. Let's modify the code above
with a different argument checking policy.

::

    Grid<double, 3, GridAssertCheck> grid(Array<int, 3>(3,3,3));

    // causes a runtime assertion failure
    grid(-1,5,10) = 1.0;

Now the program is guaranteed to stop as long as C style assertions are
enabled. Similar to the ``Grid`` class, the ``Array`` class also has two
different checking policies. Let's look at an example.

::

    Array<int, 3> a;

    // undefined behaviour
    a[5] = 1;

Again, this results in undefined behaviour and is not guaranteed to flag
up as an error. However, we can change the third template parameter of
the ``Array`` class like this.

::

    Array<int, 3, ArrayAssertArgCheck> a;

    // causes a runtime assertion failure
    a[5] = 1;

I practice it is useful to use ``typedef``\ s in a header file to enable
global switching of the argument checking policies. The following code
shows an example of how this might be done.

::

    #ifdef DEBUG_SIMULATION
    typedef GridAssertCheck GridCheck;
    typedef ArrayAssertArgCheck ArrayCheck;
    #else
    typedef GridNoArgCheck GridCheck;
    typedef ArrayNoArgCheck ArrayCheck;
    #endif

    typedef Grid<double, 3, GridCheck> SimGrid;
    typedef Array<double, 3, ArrayCheck> SimArray;

Now, by defining the macro ``DEBUG_SIMULATION`` argument checking can be
turned on globally.
