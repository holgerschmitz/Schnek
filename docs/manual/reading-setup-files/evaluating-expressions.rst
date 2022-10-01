In many cases the expressions specified in the configuration file are
evaluated right after the parsing has finished. A value is calculated
from the right hand side and assigned to the variable on the left hand
side of the assignment. In some cases it is desirable to delay the
evaluation of an expression until later. This is the case when the
values on the right hand side are not known straight away or if the
expression should be evaluated for a number of different values of a
variable appearing on the right hand side. This is often the case when
specifying a quantity for different locations in space or at different
times. Imagine, as an example, you want to define the value of a
boundary condition of the simulation as a function of time. The boundary
condition must be evaluated at each time step of the simulation. This
means that the expression will contain an independent variable that
specifies the point in time at which the boundary condition is
calculated. Let's call this independent variable ``t``. We can add an
independent variable to the The Schnek parser can be instructed to treat
a variable as read only. This means that the user will not be able to
assign a value to that variable in the configuration file. Instead, the
variable can be used on the right hand side of expressions, without
first being initialised. A read only variable can be added to the
``BlockParameters`` simply by specifying the ``readonly`` attribute. The
following example shows how this is done.

::

    class SimulationBlock : public Block {
      private:
        double t;
        pParameter paramT;
        double value;
        pParameter paramValue;

      protected:
        void initParameters(BlockParameters &parameters) {
          paramT = parameters.addParameter("t", &t, BlockParameters::readonly);
          paramValue = parameters.addParameter("value", &value);
        }
    };

In this piece of code ``t`` is added as read only variable to
``parameters`` in the ``initParameters`` method. This means that ``t``
can be used in expressions in the configuration file straight away. When
using independent variables in the input deck you should keep the object
returned by the ``addParameter`` method. This object is a smart pointer
to an object of type ``Parameter``. This object stores information about
the mathematical expression and about any dependencies of a parameter.
The ``Parameter`` objects are needed to define independent and dependent
variables in a deferred evaluation. Reading the configuration file takes
the usual form.

::

    BlockClasses blocks;
    blocks.registerBlock("sim").setClass<SimulationBlock>();

    std::ifstream in("example_setup_evaluate.setup");
    Parser P("my_simulation", "sim", blocks);
    registerCMath(P.getFunctionRegistry());
    pBlock application = P.parse(in);

    SimulationBlock &mysim = dynamic_cast<SimulationBlock&>(*application);
    mysim.evaluateParameters();

We can now write a method that iterates through values of ``t`` and
evaluates the expression supplied in the configuration file for each
value of ``t``. The following function inside the ``SimulationBlock``
class does exactly this.

::

    class SimulationBlock : public Block {
      ...
      public:
        void printValues() {
          pBlockVariables blockVars = getVariables();
          pDependencyMap depMap(new DependencyMap(blockVars));
          DependencyUpdater updater(depMap);

          updater.addIndependent(paramT);
          updater.addDependent(paramValue);


          for (int i=0; i<=20; ++i) {
            t = 0.5*i;
            updater.update();
            std::cout << t << " " << value << std::endl;
          }
        }
    };

Let's look at this code line by line.

::

    pBlockVariables blockVars = getVariables();

The ``BlockVariables`` class holds all the information about the
variables, any expressions that they depend on and that haven't yet been
evaluated. The ``Block::getVariables`` method returns a shared pointer
to the global ``BlockVariables`` object.

::

    pDependencyMap depMap(new DependencyMap(blockVars));

A ``DependencyMap`` analyses the expressions stored in the
``BlockVariables`` object. Internally it will create a data structure
that stores the dependencies of each variable. We create a new shared
pointer to a ``DependencyMap`` by passing the ``blockVars`` pointer.

::

    DependencyUpdater updater(depMap);

Finally the ``DependencyUpdater`` class can be used to create an ordered
sequence of evaluations of expressions. It will use the dependency map
to determine which expressions have to be evaluated first and which
expressions don't have to be evaluated at all in order to evaluate a
given set of dependent variables.

::

    updater.addIndependent(paramT);
    updater.addDependent(paramValue);

For the ``DependencyUpdater`` to do its job it needs to know which
variables are independent and which are dependent. This information is
supplied by calling the ``addIndependent`` and the ``addDependent``
methods. Here we pass the ``Parameter`` objects which we obtained when
adding the parameters to the ``BlockParameters`` object.

::

    for (int i=0; i<=20; ++i) {
      t = 0.5*i;
      updater.update();
      std::cout << t << " " << value << std::endl;
    }

Once the information has been set up we can change the value of ``t``
and then call ``update`` on the ``DependencyUpdater``. This will
evaluete all the expressions needed to calculate the dependent variables
and store the result in the memory locations of these dependants. This
means that after a call to ``update`` the value of the ``value``
variable will have been updated. For example, we can write the following
expression in the ``example_setup_evaluate.setup`` file.

::

    value = exp(-t/5);

This will create the following output.

::

    0 1
    0.5 0.904837
    1 0.818731
    1.5 0.740818
    2 0.67032
    2.5 0.606531
    3 0.548812
    3.5 0.496585
    4 0.449329
    4.5 0.40657
    5 0.367879
    5.5 0.332871
    6 0.301194
    6.5 0.272532
    7 0.246597
    7.5 0.22313
    8 0.201897
    8.5 0.182684
    9 0.165299
    9.5 0.149569
    10 0.135335

In this example only one expression is being evaluated. But the updater
does not care how many steps need to be taken to arrive at the result.
Consider the following input file.

::

    float decay = exp(-t/5);
    float phase = 2*t;
    float oscillation = sin(phase);
    value = oscillation*decay;

The ``DependencyUpdater`` makes sure that ``decay`` and ``phase`` are
evaluated first. ``oscillation`` depends on ``phase`` and so it will be
evaluated only after ``phase`` has been updated. Finally ``value`` is
calculated from the updated values of ``oscillation`` and ``decay``. The
result is as follows.

::

    0 0
    0.5 0.761394
    1 0.74447
    1.5 0.104544
    2 -0.5073
    2.5 -0.581617
    3 -0.153346
    3.5 0.32625
    4 0.444547
    4.5 0.167555
    5 -0.200134
    5.5 -0.332868
    6 -0.161613
    6.5 0.114509
    7 0.244281
    7.5 0.145099
    8 -0.0581267
    8.5 -0.175631
    9 -0.124137
    9.5 0.0224169
    10 0.123554

The code for this example can be downloaded
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_setup_evaluate.cpp>`__.
The setup file can be found
under \ `example\_setup\_evaluate.setup <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_setup_evaluate.setup>`__.
