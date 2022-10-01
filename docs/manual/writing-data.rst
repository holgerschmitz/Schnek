Any usable simulation code must write out the results of the simulation.
This is usually done by storing data in files. There are a number of
different file formats which are useful for different types of
applications. One of the most used format is the HDF5 file format which
supports parallel writing of data into a single file. Schnek provides a
framework to write out simulation data in HDF5 format but also allows
output in simple text format. The classes are designed so that they can
be extended for other file formats if desired. This section covers the
following topics.

-  `The Diagnostic
   Manager <http://www.notjustphysics.com/schnek/schnek-documentation/writing-data/the-diagnostic-manager/>`__
-  `Writing Grids to
   HDF5 <http://www.notjustphysics.com/schnek/schnek-documentation/writing-data/writing-grids-to-hdf5/>`__
