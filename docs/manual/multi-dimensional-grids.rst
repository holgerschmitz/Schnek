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
   Basics <http://www.notjustphysics.com/schnek/schnek-documentation/multi-dimensional-grids/grid-basics/>`__
-  `Index Types and the Array
   Class <http://www.notjustphysics.com/schnek/schnek-documentation/multi-dimensional-grids/index-types-and-the-array-class/>`__
-  `Additional Array
   Functions <http://www.notjustphysics.com/schnek/schnek-documentation/multi-dimensional-grids/additional-array-functions/>`__
-  `Argument Checking
   Policies <http://www.notjustphysics.com/schnek/schnek-documentation/multi-dimensional-grids/argument-checking-policies/>`__
-  `Sub Grids and Grid
   Transformations <http://www.notjustphysics.com/schnek/schnek-documentation/multi-dimensional-grids/sub-grids-and-grid-transformations/>`__
-  `Storage
   policies <http://www.notjustphysics.com/schnek/schnek-documentation/multi-dimensional-grids/storage-policies/>`__
