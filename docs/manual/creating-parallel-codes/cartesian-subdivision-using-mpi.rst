In the `previous
section <http://www.notjustphysics.com/schnek/schnek-documentation/creating-parallel-codes/dividing-simulation-domains/>`__
we introduced the abstract ``DomainSubdivision`` class template and
described its member functions. We already mentioned the
``MPICartSubdivision`` class that implements ``DomainSubdivision`` to
provide a Cartesian subdivision over multiple processes using MPI. In
this section we will describe a short simulation code that uses the
``MPICartSubdivision`` class to perform a simulation of the diffusion
equation. In the following we will make no use of the configuration file
parser and automatic initialisation of field data as described in
the\ `section on reading setup
files <http://www.notjustphysics.com/schnek/schnek-documentation/reading-setup-files/>`__.
We do this in order to make the code as short as possible and focus on
the use of the parallelisation routines. Also note that the numerical
scheme to simulate the diffusion equation is not very accurate and
should not be used for any real simulation. It is only included as a
very simple example. We will first write a function called
``runSimulation()`` that will contain all our simulation code. We start
by defining some parameters.

::

    void runSimulation() {
      const int N = 400;
      double dx = 2.0/N;
      double dt = 1e-3;
      Array<int,2> lo(-N/2,-N/2);
      Array<int,2> hi( N/2, N/2);

      Array<double,2> domainLo(-1.0, -1.0);
      Array<double,2> domainHi( 1.0,  1.0);

      Range<double, 2> domain(domainLo, domainHi);
      Array<bool, 2> stagger(false, false);
     
      // ...

In this piece of code, ``N`` is the size of the grid, ``dx`` is the
physical extent of a single grid cell and ``dt`` is the time step. The
``lo`` and ``hi`` arrays will be used to define the index range of our
grid. We will actually be using the ``Field`` class to define the grid.
As explained in the `section on
fields <http://www.notjustphysics.com/schnek/schnek-documentation/ranges-ghost-cells-and-fields/fields/>`__,
this class also needs a physical extent, a definition of the grid
staggering and the number of ghost cells. ``domainHi`` and ``domainLo``
are used to define the physical extent ``domain``, and ``stagger`` is
initialised to say that our grid is not staggered in any direction. We
can now create an ``MPICartSubdivision`` object, initialise it with the
global grid size and then obtain the information about the local grid.

::

      MPICartSubdivision<Field<double, 2> > subdivision;

      subdivision.init(lo, hi, 1);

      Array<int,2> localLo = subdivision.getInnerLo();
      Array<int,2> localHi = subdivision.getInnerHi();
      Range<double, 2> localDomain = subdivision.getInnerExtent(domain);

The template argument for the ``MPICartSubdivision`` class is the type
of field that we want to exchange between processes. A call to the
``init()`` function sets the global index range for the grids,
**excluding any ghost cells**. The third parameter to the ``init()``
function is the number of ghost cells. In our case this is 1. After
initialisation we can obtain the local index range **excluding any ghost
cells** usnig the ``getInnerLo()`` and ``getInnerHi()`` functions. The
local physical domain is obtained by calling ``getInnerExtent()`` and
passing the global physical extent. The ``Field`` is created using these
local ranges.

::

      Field<double, 2> field(localLo, localHi, localDomain, stagger, 1);

Now we can initialise the field with some values.

::

      for (int i=localLo[0]; i<=localHi[0]; ++i)
        for (int j=localLo[1]; j<=localHi[1]; ++j) {
          double x = field.indexToPosition(0,i);
          double y = field.indexToPosition(1,j);
          field(i,j) = exp(-25*(x*x + y*y));
        }
      
      subdivision.exchange(field);

Notice how we are using ``localLo`` and ``localHi`` as limits for the
loops. This means that this loop will iterate only over the inner cells
of the field, an exclude the ghost cells. The ``indexToPosition()``
member function of the ``Field`` class converts the indices into
physical positions. After the field values have been set we issue a call
to ``exchange()`` on the ``MPICartSubdivision`` object. This call
exchanges ghost cell data between processes. After this call the ghost
cells of the ``field`` variable will have been filled with the data from
the neighbouring processes. The next step is to implement the simulation
loop.

::

      for (int t=0; t<1000; ++t) {
      
        for (int i=localLo[0]; i<=localHi[0]; ++i)
          for (int j=localLo[1]; j<=localHi[1]; j+=2) {
            field(i,j) = field(i,j)
                + dt*(field(i-1,j) + field(i+1,j)
                    + field(i,j-1) + field(i,j+1) - 4*field(i,j));
          }

        subdivision.exchange(field);
      }

For simplicity the simulation loop is executed 1000 times. Again, for
the field update we use ``localLo`` and ``localHi`` as limits for the
loops. But notice how we are looking up values of the neighbouring grid
cells with the indices ``i-1``, ``i+1``, ``j-1``, and ``j+1``. When the
indices are on the limits this results in a request for values outside
the domain given by ``localLo`` and ``localHi``. This is OK because
``localLo`` and ``localHi`` specify the inner domain, excluding the
ghost cells. When initialising the field we have specified that one
ghost cell should be added. So we are allowed to look outside of the
inner range by one grid cell. After the field values have been updated
we call ``exchange()`` on the ``MPICartSubdivision`` object again to
exchange ghost cell data between processes. This keeps the ghost cells
up-to-date for the next iteration of the simulation loop. We are
omitting any output routines from this example. The sections on
diagnostic output will cover on how to save data from multiple processes
into HDF5 files. The code for this example `can be found
here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_mpisub.cpp>`__.
