Basics
~~~~~~

The format of the setup files that Schnek can read is a structured
format containing blocks of parameters. Each type of block corresponds
to a class in the simulation code and each definition of a block in the
setup file will create an object of the corresponding class. To create a
simulation block, simply create a class that extends ``Block``.

::

    class SimulationBlock : public Block
    {};

In order to read an input file and create an object of the new class
``SimulationBlock`` you need to register the class with a
``BlockClasses`` object. In your main function you can specify the
following.

::

      BlockClasses blocks;
      blocks.addBlock("sim");
      blocks("sim").setClass<SimulationBlock>();

The first line creates a ``BlockClasses`` object. The second line
registers a new block name called ``"sim"``. You can choose whatever
name you want here. The third line links the block name ``"sim"`` with
the ``SimulationBlock`` class. This line is needed by the parser to
create the correct objects from the setup file. The last two lines of
the code above can be abbreviated.

::

      blocks.addBlock("sim").setClass<SimulationBlock>();

Now that we have registered the simulation blocks, we can use the parser
to read a setup file.

::

      std::ifstream in("example.setup");
      
      Parser P("my_simulation", "sim", blocks);
      pBlock application = P.parse(in);

The first line simply opens a file called ``example.setup``. The second
line creates a ``Parser`` object by specifying three parameters.
``"my_simulation"`` is simply the name of the simulation that you want
to run. You can set this to anything you like. ``"sim"`` is the block
name of the root block. This means that the contents of the file are
interpreted as belonging to the ``"sim"`` block. ``blocks`` is the
``BlockClasses`` object that specifies the available block names and the
types that are associated with them. The last line reads the setup file
from the input stream and returns a ``boost::shared_ptr`` to a ``Block``
object. This will be of the type specified by the root block. You can
safely cast it to the class ``SimulationBlock`` class which has been
linked to the ``"sim"`` root block.

::

      SimulationBlock &mysim = dynamic_cast<SimulationBlock&>(*application);

Reading Values
~~~~~~~~~~~~~~

Until now, we have only created a ``SimulationBlock`` object from some
setup file. We haven't even specified what the setup file should look
like. That's because we are not reading in any values from the file. But
our simulation block will contain parameters that you will want to
specify in the setup file. It turns out that it is actually quite easy
to register parameters and read values into them. Let's extend the
definition of our ``SimulationBlock`` class.

::

    class SimulationBlock : public Block
    {
      private:
        int size;
        double dx;
        std::string info;

      protected:
        void initParameters(BlockParameters &parameters)
        {
          parameters.addParameter("size", &size);
          parameters.addParameter("dx", &dx);
          parameters.addParameter("info", &info);
        }
    };

We have defined three parameters called ``size``, ``dx``, and ``info``.
The three parameters are of type ``int``, ``double``, and
``std::string``. These are the three types currently supported by
Schnek. In addition to declaring the class parameters we have
implemented a method called ``initParameters``. This method is a virtual
method defined by the ``Block`` class. It is called before the setup
file is read and is used to register all the parameters. Calling
``addParameter`` on the ``BlockParameters`` object thath is passed into
the method registers the parameters. The ``addParameter`` method takes
two arguments. The first argument is the name of the parameter as it
would appear in the setup file. The second argument is the memory
address of the variable that the value should be assigned to. Once we
have registered all the parameters, all we have to do is to make sure
that the values are evaluated from the setup file. This is achieved by a
call to ``evaluateParameters`` on the root simulation block.

::

      mysim.evaluateParameters();

The Setup File
~~~~~~~~~~~~~~

Now that we have written the basic code for reading setup files, let's
take a look at the format of the setup file itself. Some possible
contents of ``example.setup`` are as follows.

::

    size = 123;
    dx = 3.056;
    info = "A test setup";

The format somewhat resemble C-style syntax. Lines are terminated with
semicolons and whitespaces are ignored. In the setup file you can
directly assign values to the parameters registered in the root block of
your simulation. But the format of the setup file is a lot more
versatile than this. Let's look at the following example.

::

    float pihalf = 3.14159/2.0;
    int counter = 7;

    size = counter^2;
    dx = 3.056*pihalf;
    info = "data"+counter+".out";

You can define your own variables in the setup file and use them to
calculate the parameters. The basic operations addition, subtraction,
multiplication, division and exponentiation are supplied out of the box.
The input file above will result in the parameters of the root
``SimulationBlock`` object to be set to the following values.

::

    size: 49
    dx: 4.80035
    info: data7.out

Adding Mathematical Functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Let's go one step further and register all the mathematical function in
the standard ``cmath`` library. To do this, we have to modify our code
by adding a single line before parsing the setup file.

::

      Parser P("my_simulation", "sim", blocks);
      registerCMath(P.getFunctionRegistry());
      pBlock application = P.parse(in);

Now we can change the setup file as follows.

::

    float pihalf = 3.14159/2;
    int counter = 7;

    size = counter^2;
    dx = sin(pihalf*0.5)*exp(-1.5^2);
    info = "data"+counter+".out";

Note how we have used the ``sin`` and ``exp`` functions. The resulting
values assigned to the ``SimulationBlock`` members are as follows.

::

    size: 49
    dx: 0.0745285
    info: data7.out

The code for this tutorial can be found
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_setup_first.cpp>`__,
the input files are
```example_setup_first01.setup`` <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_setup_first01.setup>`__
```example_setup_first02.setup`` <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_setup_first02.setup>`__
```example_setup_first03.setup`` <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_setup_first03.setup>`__
