Schnek provides a simplified interface for creating parallel simulation
codes. At the heart of this interface lies the \ ``DomainSubdivision``
class. This class defines an abstract interface for partitioning a
global simulation domain into local sub-domains for the individual
processes. Implementations of this interface use MPI routines to pass
data between the processes. This section covers the following topics.

-  `Dividing the simulation
   domain <http://www.notjustphysics.com/schnek/schnek-documentation/creating-parallel-codes/dividing-simulation-domains/>`__
-  `Cartesian subdivision using
   MPI <http://www.notjustphysics.com/schnek/schnek-documentation/creating-parallel-codes/cartesian-subdivision-using-mpi/>`__
