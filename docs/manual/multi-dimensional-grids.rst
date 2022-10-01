C++ has only very rudimentary support for multi-dimensional arrays.
Schnek contains a ``Grid`` class that allows multi-dimensional arrays to
be easily created and resized. The Grid class is strongly customisable
using template parameters. It allows different storage policies. For
example, the Lazy Storage Policy can increase performance for grids that
are resized often. There are also different policies for checking the
range of the index arguments passed to the Grid. During development you
might want to switch argument checking on but for the production version
of your code, when speed is important, you can switch argument checking
off. This section covers the following topics.

-  `Grid
   Basics <manual/multi-dimensional-grids/grid-basics.html>`__
-  `Index Types and the Array
   Class <manual/multi-dimensional-grids/index-types-and-the-array-class.html>`__
-  `Additional Array
   Functions <manual/multi-dimensional-grids/additional-array-functions.html>`__
-  `Argument Checking
   Policies <manual/multi-dimensional-grids/argument-checking-policies.html>`__
-  `Sub Grids and Grid
   Transformations <manual/multi-dimensional-grids/sub-grids-and-grid-transformations.html>`__
-  `Storage
   policies <manual/multi-dimensional-grids/storage-policies.html>`__
