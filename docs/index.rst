.. Schnek documentation master file, created by
   sphinx-quickstart on Sat Dec 11 12:43:52 2021.


Schnek Documentation
==================================

Schnek is a C++ library that supports the development of
large parallel simulation codes, originally intended for physics
applications. The objective of the library to keep the user code simple
while allowing flexibility and performance. The library is intended to
be compatible with most \*nix flavours. Schnek is heavily reliant of the
`Boost C++ library <http://www.boost.org/>`__. In most cases it will
also be compiled with support for `MPI <http://www.open-mpi.org/>`__ and
`HDF5 <http://www.hdfgroup.org/HDF5/>`__, but it can run without these.

The tutorial pages will cover the following topics.

-  `Installation <./manual/installation.html>`__
-  `Multi-Dimensional Grids <./manual/multi-dimensional-grids.html>`__

   -  `Grid Basics <./manual/multi-dimensional-grids/grid-basics.html>`__
   -  `Index Types and the Array Class <./manual/multi-dimensional-grids/index-types-and-the-array-class.html>`__
   -  `Additional Array Functions <./manual/multi-dimensional-grids/additional-array-functions.html>`__
   -  `Argument Checking Policies <./manual/multi-dimensional-grids/argument-checking-policies.html>`__
   -  `Sub Grids and Grid Transformations <./manual/multi-dimensional-grids/sub-grids-and-grid-transformations.html>`__
   -  `Storage policies <./manual/multi-dimensional-grids/storage-policies.html>`__

-  `Reading Setup
   Files <./manual/reading-setup-files.html>`__

   -  `A first example <./manual/reading-setup-files/a-first-example.html>`__
   -  `Error Handling <./manual/reading-setup-files/error-handling.html>`__
   -  `Default Values, Arrays and Constants <./manual/reading-setup-files/default-values-arrays-and-constants.html>`__
   -  `Evaluating expressions <./manual/reading-setup-files/evaluating-expressions.html>`__
   -  `Filling fields <./manual/reading-setup-files/filling-fields.html>`__
   -  `Adding user defined functions <./manual/reading-setup-files/adding-user-defined-functions.html>`__

-  `Simulation Blocks and Data <./manual/simulation-blocks-and-data.html>`__

   -  `Hierarchical Simulation Blocks <./manual/simulation-blocks-and-data/hierarchical-simulation-blocks.html>`__
   -  `Handling Multiple Child Block Types <./manual/simulation-blocks-and-data/handling-multiple-child-block-types.html>`__
   -  `Sharing Data Between Blocks <./manual/simulation-blocks-and-data/sharing-data-between-blocks.html>`__
   -  `The Block Initialisation Life Cycle <./manual/simulation-blocks-and-data/the-block-initialisation-life-cycle.html>`__
   -  `Adding Literature References <./manual/simulation-blocks-and-data/adding-literature-references.html>`__

-  `Ranges, Ghost Cells and Fields <./manual/ranges-ghost-cells-and-fields.html>`__

   -  `Rectangular Ranges <./manual/ranges-ghost-cells-and-fields/rectangular-ranges.html>`__
   -  `Boundaries <./manual/ranges-ghost-cells-and-fields/boundaries.html>`__
   -  `Fields <./manual/ranges-ghost-cells-and-fields/fields.html>`__

-  `Creating Parallel Codes <./manual/creating-parallel-codes.html>`__

   -  `Dividing the Simulation Domain <./manual/creating-parallel-codes/dividing-simulation-domains.html>`__
   -  `Cartesian subdivision using MPI <./manual/creating-parallel-codes/cartesian-subdivision-using-mpi.html>`__

-  `Writing Data <./manual/writing-data.html>`__

   -  `The Diagnostic Manager <./manual/writing-data/the-diagnostic-manager.html>`__
   -  `Writing Grids to HDF5 <./manual/writing-data/writing-grids-to-hdf5.html>`__

.. |SchnekLogo| image:: http://www.notjustphysics.com/wp-content/uploads/2014/06/SchnekLogo.png
   :class: alignleft wp-image-127 size-full
   :width: 200px
   :height: 200px
   :target: http://www.notjustphysics.com/wp-content/uploads/2014/06/SchnekLogo.png

API Reference
==================

* `Overview <./html/index.html>`_
* `Class Index <./html/annotated.html>`_
* `File List <./html/files.html>`_
