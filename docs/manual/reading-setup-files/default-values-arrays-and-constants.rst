Default Values
~~~~~~~~~~~~~~

When initialising a simulation from a configuration file it is, in many
cases, desirable to specify reasonable default values for some of the
parameters. This means that the end user does not have to specify every
last value. This helps keep configuration files manageable. Recall the
code from the First Example.

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

The corresponding configuration file needs to specify values for all
three parameters. If you omit only one of them you will get an error
when trying to evaluate the parameters with a call to
``evaluateParameters``. However, Schnek let's you easily define default
values for the parameters by simply passing them to the ``addParameter``
function. The following code shows how.

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
          parameters.addParameter("size", &size, 100);
          parameters.addParameter("dx", &dx, 0.01);
          parameters.addParameter("info", &info, std::string("No information supplied."));
        }
    };

Now you are free to leave any of the three parameters from the
configuration file in which case the default values will be set. The
code for this example can be found
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_setup_defaults.cpp>`__.
An example setup file is given by
`example\_setup\_defaults01.setup <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_setup_defaults01.setup>`__.

Arrays
~~~~~~

If some of your parameters are arrays, Schnek provides an easy to use
function for adding those as well. Look at the following example.

::

    class SimulationBlock : public Block {
      private:
        Array<double, 3> pos;

      protected:
        void initParameters(BlockParameters &parameters) {
          parameters.addArrayParameter("pos", pos);
        }
    };

This code will register three individual parameters ``posx``, ``posy``
and ``posy`` for the three components of the array. The corresponding
configuration file will look like this.

::

    posx = 0.5;
    posy = 1.5;
    posz = 2.5;

Of course you can specify a default value that you want the array to be
filled with if no value is specified in the configuration file.
Modifying the line in the code above to

::

          parameters.addArrayParameter("pos", pos, -1.0);

will set the default value to ``-1.0``. The configuration file is now
free to specify only some of the values.

::

    posx = 0.5;
    posz = 2.5;

This will result in the array being initialised with the following
values.

::

    pos = (0.5, -1.0, 2.5)

The code for this example is available
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_setup_array.cpp>`__.
An example setup file is given by
`example\_setup\_array.setup <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_setup_array.setup>`__.

Constants
~~~~~~~~~

Finally, you might want to help the user of your simulation code to
quickly create configuration files by supplying useful constants. These
values should be available to be used inside expressions. On the other
hand the user should not be able to modify the values of these
constants. The ``addConstant`` function does exactly that. Given the
code above you can simply add the following line to the
``initParameters`` function.

::

    parameters.addConstant("pi", M_PI);

Assuming that ``M_PI`` is defined and holds the value of π, this will
allow the use of ``pi`` in the configuration file, such as

::

    dx = 2*pi;

Attempting to change the value of ``pi`` on the other hand will result
in an error. The code for this example is available
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_setup_constant.cpp>`__.
An example setup file is given
by \ `example\_setup\_constant.setup <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_setup_constant.setup>`__.
