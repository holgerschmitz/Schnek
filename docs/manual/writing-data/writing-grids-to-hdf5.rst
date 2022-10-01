One of the most popular file formats for writing numerical data is the
HDF5 format. This format is widely supported by visualisation tools and
can be read by many other software including Matlab and others. Schnek
supports writing data in HDF5 using parallel file output. This means
that each process in a parallel execution environment writes its own
piece of data into a single file. Creating parallel HDF5 diagnostic in
Schnek is relatively simple. First of all you need to create a class
that inherits from ``HDFGridDiagnostic`` and implements two methods.

::

    typedef Field<double, 2> SimField;

    Array<int,2> lo(-N/2,-N/2);
    Array<int,2> hi( N/2, N/2);

    class Diagnostic : public HDFGridDiagnostic<SimField, SimField*, DeltaTimeDiagnostic>
    {
      protected:
        Array<int,2> getGlobalMin() {
          return lo;
        }

        Array<int,2> getGlobalMax() {
          return hi;
        }
    };

The ``HDFGridDiagnostic`` class takes three arguments. The first
argument is the ``Field`` or ``Grid`` type that should be written out.
Note that specifying a ``Field`` type will automatically take care of
the ghost cells of the field. ``HDFGridDiagnostic`` obtains the grid for
output through the ``Block``'s ``retrieveData()`` function (see the
section on `sharing data between
blocks <http://www.notjustphysics.com/schnek/schnek-documentation/simulation-blocks-and-data/sharing-data-between-blocks/>`__).
The second argument is the pointer type of the field that should be
retrieved from the ``Block``. The third template argument specifies
whether the diagnostic should use the physical time or the integer time
step to determine when data should be written. The choices here are
``DeltaTimeDiagnostic`` and ``IntervalDiagnostic``. Two functions need
to be implemented. The ``getGlobalMin()`` and ``getGlobalMax()``
function should return the global extent of the grid across all
processes. This range should not include any ghost cells. Once the class
has been defined it can be included in the simulation `block hierarchy
for the setup file
parser <http://www.notjustphysics.com/schnek/schnek-documentation/simulation-blocks-and-data/hierarchical-simulation-blocks/>`__.

::

    blocks.registerBlock("Simulation").setClass<Simulation>();
    blocks.registerBlock("Diagnostic").setClass<Diagnostic>();

    blocks("Simulation").addChildren("Diagnostic");

During the initialisation of the blocks, the field that should be
available to the HDF5 diagnostic must be added to the ``Block`` shared
data. **Note** that the ``HDFGridDiagnostic`` class sets up its internal
parameters for writing the HDF5 files during the ``init()`` phase of
block initialisation. This means that the grid or field must have been
added and properly resized before the ``init()`` function is called. It
is advisable to do this during the ``preInit()`` phase (see the `section
on block
initialisation <http://www.notjustphysics.com/schnek/schnek-documentation/simulation-blocks-and-data/the-block-initialisation-life-cycle/>`__).

::

    Field<double, 2> field(localLo, localHi, localDomain, stagger, 1);

    addData("field", field);

Here ``localLo`` and ``localHi`` defines the local extent of the grid or
field. Finally we need to call the ``DiagnosticManager`` to write out
the fields when needed.

::

    DiagnosticManager::instance().setPhysicalTime(&time);

    while (time <= simulationEndTime) {
      doSimulationStep();
      
      time += dt;
      
      DiagnosticManager::instance().execute();
    }

See the `previous section on the
``DiagnosticManager`` <http://www.notjustphysics.com/schnek/schnek-documentation/writing-data/the-diagnostic-manager/>`__
for details. Now the diagnostic output can be controlled through the
setup file. A typical entry in the setup file for writing a field would
be the following.

::

    Diagnostic Field {
      file = "Field_#t.h5";
      field = "field";
      deltaTime = 0.2;
    }

First of all, note the ``#t`` token in the file name. This character
sequence ``#t`` will be replaced with the count of the diagnostic
output. This means that the code will end up generating output files
named

::

    Field_0.h5
    Field_1.h5
    Field_2.h5
    Field_3.h5
    Field_4.h5
    Field_5.h5

The ``field = "field";`` line specifies the field that should be
written. The string on the right hand side corresponds to the string
passed to the ``addData()`` function in the code. Usually there are
multiple data field in the code, such as the following.

::

    addData("Ex", Ex);
    addData("Ey", Ey);
    addData("Ez", Ez);

To generate HDF5 output for each field you would add separate diagnostic
blocks in the setup file.

::

    Diagnostic Ex {
      file = "Ex_#t.h5";
      field = "Ex";
      deltaTime = 0.2;
    }
    Diagnostic Ey {
      file = "Ey_#t.h5";
      field = "Ey";
      deltaTime = 0.2;
    }
    Diagnostic Ez {
      file = "Ez_#t.h5";
      field = "Ez";
      deltaTime = 0.2;
    }

Finally ``deltaTime`` specifies the physical time interval after which
an output should be generated. For ``IntervalDiagnostic`` the number of
tie steps is specified using ``interval`` which is illustrated in the
following snippet.

::

    IntervalDiagnostic Field {
      file = "Field_#t.h5";
      field = "field";
      interval = 100;
    }

A minimal example demonstrating the ``HDFGridDiagnostic`` can be
`downloaded
here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_diagnostic_hdf5.cpp>`__.
The setup file can be found
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_diagnostic_hdf5.setup>`__
