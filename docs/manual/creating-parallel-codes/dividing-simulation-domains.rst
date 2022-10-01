When running parallel simulations on Cartesian grids the grid data has
to be distributed among the multiple processes. In many cases a
Cartesian decomposition of the grid into multiple rectangular sub-grids
is the easiest method of splitting the simulation domain. This simple
technique gives good performance on large number of processes for many
problems. Schnek provides classes for dividing the global domain in this
way. However, the framework can be extended to include more elaborate
decomposition techniques, as long as the individual local domains remain
rectangular. In the \ `previous
chapter <http://www.notjustphysics.com/schnek/schnek-documentation/ranges-ghost-cells-and-fields/boundaries/>`__ we
have seen how the \ ``Boundary`` class can be used to specify the inner
domain and the ghost cells of a local simulation domain.
The \ ``DomainSubdivision`` class framework performs the task of
actually splitting a large simulation domain. \ ``DomainSubdivision`` is
an abstract class that provides an interface for defining local domains
and exchanging data between domains. Specific implementations will
define these operations for different scenarios. Two subclasses
of \ ``DomainSubdivision`` are available in Schnek.

SerialSubdivision
    A subdivision class that works for single processor, serial codes.
    This class is useful for codes that should run on serial and
    parallel architectures.
MPICartSubdivision
    A subdivision class that uses MPI to split the global domain into
    rectangular regions of equal size.

Initialisation
~~~~~~~~~~~~~~

The \ ``DomainSubdivision`` is templated with a \ ``GridType`` class
that should correspond to a specific \ ``Grid`` class. In the
following \ ``rank`` is used to indicate the rank of the
grid. \ ``DomainSubdivision`` defines a default constructor and a set of
initialisation methods.

::

      DomainSubdivision();
      virtual void init(const Array<int,rank> &low, 
                        const Array<int,rank> &high, 
                        int delta) = 0;
      void init(const Range<int, rank> &domain, int delta);
      void init(const GridType &grid, int delta);
      void init(const Array<int,rank> &size, int delta);

The constructor creates an empty subdivision object. The object needs to
be initialised using one of the \ ``init`` methods before it can be
used. The first of these methods is intended to actually initialise
the \ ``DomainSubdivision`` object. The arguments passed to the method
are the limits of the global simulation domain, \ ``low`` and ``high``,
including any ghost cells. The argument \ ``delta`` specifies the
thickness of the boundary layer. This parameter is discussed in detail
in the manual of the \ ``Boundary`` class. The other
three \ ``init`` methods are convenience methods that forward the call
to the first \ ``init`` method.

::

      void init(const Range<int, rank> &domain, int delta) {
        init(domain.getLo(), domain.getHi(), delta);
      }

This method takes a range and delta and then initialises the global
domain with the lower and upper bound of the range.

::

      void init(const GridType &grid, int delta) {
        init(grid.getLo(), grid.getHi(), delta);
      }

This method takes a grid and uses the bounds of the grid to create a
global domain of the same size.

::

      void init(const Array<int,rank> &size, int delta);

This method takes the size of the grid and creates a global domain that
ranges from 0 to size-1.

Local Domain Boundaries
~~~~~~~~~~~~~~~~~~~~~~~

After the \ ``DomainSubdivision`` has been initialised it should have
calculated the local domain boundaries for the current process. For any
parallel process, these domains should be distinct and the ghost cells
of one domain should map onto the inner cells of some other domain. The
following methods return the dimensions of the local domain and its
ghost domains.

::

       
      const Range<int, rank> &getDomain() const;
      const Array<int,rank> &getLo() const;
      const Array<int,rank> &getHi() const;

The method \ ``getDomain`` returns the local domain including any ghost
cells. This domain should be used for initialising local grids.
Individual access to the lower and upper bounds are provided by the two
convenience methods \ ``getLo`` and ``getHi``.

::

      Range<int, rank> getInnerDomain() const;
      Array<int,rank> getInnerLo() const;
      Array<int,rank> getInnerHi() const;

The method \ ``getInnerDomain`` returns the local inner domain. This
domain can be used for iterating over the local grid do perform any
calculations. Individual access to the lower and upper bounds are
provided by the two convenience
methods \ ``getInnerLo`` and ``getInnerHi``.

Passing Data Between Processes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The \ ``DomainSubdivision`` class defines a few abstract methods that
allow passing data between precesses. In this way it provides an
abstraction that allows different parallelisation methods and different
domain decompositions to use the same interface.

::

        virtual void exchange(GridType &grid, int dim) = 0;

``exchange`` exchanges grid data between the processes of a parallel
simulation. The ghost cells of one local domain are filled with data
from the ghost source domain of the neighbouring process. The
implementation should ensure that data is wrapped around the global
simulation domain. The argument \ ``dim`` specifies the direction of the
exchange. To exchange other types of data you can use
the \ ``exchangeData`` method.

::

      typedef Grid BufferType;
      virtual void exchangeData(int dim, int orientation, BufferType &in, BufferType &out) = 0;

This method exchanges a one dimensional data buffer between neighbouring
processes. The is carried out only with one neighbour, given
by \ ``orientation``, where a positive orientation indicates an exchange
sending to the upper neighbour and recieving from the lower neighbour,
and a negative orientation indicates communication the other way.
**Note** that the \ ``buffer type``\ BufferType defined in
the \ ``DomainSubdivision`` class is a one dimensional grid using lazy
storage policy. This means that you can freely resize the buffer to fit
the data that needs to be sent. This will not incur a large amount of
time penalty for deallocating and allocating memory as long as the size
of the buffer does not fluctuate too rapidly. The following three
methods allow a global data reduction for single values.

::

      virtual double sumReduce(double) const = 0;
      virtual double maxReduce(double) const = 0;
      virtual double avgReduce(double) const = 0;

``sumReduce`` will calculate the sum of all the values passed to it from
all the processes, while \ ``maxReduce`` will calculate the maximum
value. The method \ ``avgReduce`` will calculate the average value over
all the processes.

Utility Methods
~~~~~~~~~~~~~~~

A number of methods are provided to enable you to locate the current
process.

::

      virtual bool master() const = 0;
      virtual int procCount() const = 0;
      virtual int procnum() const = 0;

For every parallel simulation, one process is declared as the master
process. The method \ ``master`` returns true in the master process and
false for every other process. This is useful for user level logging.
The method \ ``procCount`` returns the total number of processes
and \ ``procnum`` returns the number of the current process. Note that
the number given by \ ``procnum`` is not guaranteed to remain the same
from one run to another for the same local sub-domain. It could depend
on how the processes are distributed among the nodes of a parallel
computer architecture. In order to obtain a unique identifier, that
reflects the location of the process and will not change between
simulation runs you should use \ ``getUniqueId``

::

      virtual int getUniqueId() const = 0;

Finally, the following two methods allow to inspect whether the local
process is at the upper or lower edge of the global simulation domain.

::

      virtual bool isBoundLo(int dim) = 0;
      virtual bool isBoundHi(int dim) = 0;

``isBoundLo`` will return true is the local domain is at the lower edge
of the global simulation domain in the direction given by \ ``dim``.
Similarly, \ ``isBoundHi`` will return true is the local domain is at
the upper edge of the global simulation domain. These two methods should
be used when applying global boundary conditions to the grid.
