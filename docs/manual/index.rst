.. _manual-index:

Schnek is a C++ library that supports the development of
large parallel simulation codes, originally intended for physics
applications. The objective of the library to keep the user code simple
while allowing flexibility and performance. The library is intended to
be compatible with most \*nix flavours. Schnek is heavily reliant of the
`Boost C++ library <http://www.boost.org/>`__. In most cases it will
also be compiled with support for `MPI <http://www.open-mpi.org/>`__ and
`HDF5 <http://www.hdfgroup.org/HDF5/>`__, but it can run without these.

The tutorial pages will cover the following topics.

-  `Installation <http://www.notjustphysics.com/schnek/schnek-documentation/installation/>`__
-  `Multi-Dimensional
   Grids <http://www.notjustphysics.com/schnek/schnek-documentation/multi-dimensional-grids/>`__

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

-  `Reading Setup
   Files <http://www.notjustphysics.com/schnek/schnek-documentation/reading-setup-files/>`__

   -  `A first
      example <http://www.notjustphysics.com/schnek/schnek-documentation/reading-setup-files/a-first-example/>`__
   -  `Error
      Handling <http://www.notjustphysics.com/schnek/schnek-documentation/reading-setup-files/error-handling/>`__
   -  `Default Values, Arrays and
      Constants <http://www.notjustphysics.com/schnek/schnek-documentation/reading-setup-files/default-values-arrays-and-constants/>`__
   -  `Evaluating
      expressions <http://www.notjustphysics.com/schnek/schnek-documentation/reading-setup-files/evaluating-expressions/>`__
   -  `Filling fields <http://www.notjustphysics.com/schnek/schnek-documentation/reading-setup-files/filling-fields/>`__
   -  `Adding user defined
      functions <http://www.notjustphysics.com/schnek/schnek-documentation/reading-setup-files/adding-user-defined-functions/>`__

-  `Simulation Blocks and
   Data <http://www.notjustphysics.com/schnek/schnek-documentation/simulation-blocks-and-data/>`__

   -  `Hierarchical Simulation
      Blocks <http://www.notjustphysics.com/schnek/schnek-documentation/simulation-blocks-and-data/hierarchical-simulation-blocks/>`__
   -  `Handling Multiple Child Block
      Types <http://www.notjustphysics.com/schnek/schnek-documentation/simulation-blocks-and-data/handling-multiple-child-block-types/>`__
   -  `Sharing Data Between
      Blocks <http://www.notjustphysics.com/schnek/schnek-documentation/simulation-blocks-and-data/sharing-data-between-blocks/>`__
   -  `The Block Initialisation Life
      Cycle <http://www.notjustphysics.com/schnek/schnek-documentation/simulation-blocks-and-data/the-block-initialisation-life-cycle/>`__
   -  `Adding Literature
      References <http://www.notjustphysics.com/schnek/schnek-documentation/simulation-blocks-and-data/adding-literature-references/>`__

-  `Ranges, Ghost Cells and
   Fields <http://www.notjustphysics.com/schnek/schnek-documentation/ranges-ghost-cells-and-fields/>`__

   -  `Rectangular Ranges <http://www.notjustphysics.com/schnek/schnek-documentation/ranges-ghost-cells-and-fields/rectangular-ranges/>`__
   -  `Boundaries <http://www.notjustphysics.com/schnek/schnek-documentation/ranges-ghost-cells-and-fields/boundaries/>`__
   -  `Fields <http://www.notjustphysics.com/schnek/schnek-documentation/ranges-ghost-cells-and-fields/fields/>`__

-  `Creating Parallel
   Codes <http://www.notjustphysics.com/schnek/schnek-documentation/creating-parallel-codes/>`__

   -  `Dividing the Simulation
      Domain <http://www.notjustphysics.com/schnek/schnek-documentation/creating-parallel-codes/dividing-simulation-domains/>`__
   -  `Cartesian subdivision using
      MPI <http://www.notjustphysics.com/schnek/schnek-documentation/creating-parallel-codes/cartesian-subdivision-using-mpi/>`__

-  `Writing
   Data <http://www.notjustphysics.com/schnek/schnek-documentation/writing-data/>`__

   -  `The Diagnostic
      Manager <http://www.notjustphysics.com/schnek/schnek-documentation/writing-data/the-diagnostic-manager/>`__
   -  `Writing Grids to
      HDF5 <http://www.notjustphysics.com/schnek/schnek-documentation/writing-data/writing-grids-to-hdf5/>`__

.. |SchnekLogo| image:: http://www.notjustphysics.com/wp-content/uploads/2014/06/SchnekLogo.png
   :class: alignleft wp-image-127 size-full
   :width: 200px
   :height: 200px
   :target: http://www.notjustphysics.com/wp-content/uploads/2014/06/SchnekLogo.png
