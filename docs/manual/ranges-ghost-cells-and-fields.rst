One very useful feature of Schnek is the ability to define rectangular
domains and ghost cells on numerical grids. Ghost cells are grid cells
that lie outside of the inner simulation domain of a simulation process.
They can serve two different purposes. On the outer edge of the
simulation domain they are needed in order to enforce boundary
conditions. At the edges between the simulation domain of two processes
they represent values that are being calculated on the neighbouring
process. The Field class in Schnek is a Grid with additional information
attached to it. It handles ghost cells, and it is aware of the physical
dimensions that the grid represents in the real world. Utility methods
allow the mapping of grid indices to real world coordinates and back,
while allowing for grid staggering. This section covers the following
topics.

-  `Rectangular
   Ranges <manual/ranges-ghost-cells-and-fields/rectangular-ranges.html>`__
-  `Boundaries <manual/ranges-ghost-cells-and-fields/boundaries.html>`__
-  `Fields <manual/ranges-ghost-cells-and-fields/fields.html>`__
