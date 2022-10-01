One of the great strengths of Schnek is the ability to extend the setup
file parser with user defined functions. As we already introduced in a
`previous
example <http://www.notjustphysichttps/schnek-documentation/reading-setup-files/a-first-example/>`__
adding all the functions from the ``cmath`` library is done with a
single line of code. It turns out that adding your own custom functions
is just as simple. Let's start again creating a simulation block with
two variables

::

    class SimulationBlock : public Block {
      private:
        double value;
        int n;
      protected:
        void initParameters(BlockParameters &parameters) {
          parameters.addParameter("value", &value);
          parameters.addParameter("n", &n);
        }

      public:
        void writeValues() {
          std::cout << "value = " << value << std::endl;
          std::cout << "n = " << n << std::endl;
        }
    };

The variable ``value`` is a ``double`` and ``n`` is an integer. As
before we have two functions. ``initParameters()`` registers the
variables with the parser and ``writeValues`` simply writes out the
values of the two variables. The code for configuring and starting up
the parser is also almost the same as we have seen before.

::

    int main() {
      BlockClasses blocks;

      blocks.registerBlock("sim").setClass<SimulationBlock>();

      std::ifstream in("example_setup_functions.setup");

      Parser P("my_simulation", "sim", blocks);

      registerCMath(P.getFunctionRegistry());
      
      // ADDITIONAL FUNCTIONS WILL BE REGISTERED HERE

      try {
        pBlock application = P.parse(in);

        SimulationBlock &mysim = dynamic_cast<SimulationBlock&>(*application);
        mysim.evaluateParameters();
        mysim.writeValues();
      }
      catch (ParserError &e) {
        std::cerr << "Parse error in " << e.getFilename() << " at line " << e.getLine() << ": "<< e.message << std::endl;
        exit(-1);
      }
      catch (EvaluationException &e) {
        std::cerr << "Evaluation Error: " << e.getMessage() << std::endl;
        exit(-1);
      }

      return 0;
    }

Note here the comment after the call to ``registerCMath()``. This
comment indicates the place where we will insert the code for
registering our custom functions. Before we do that, we need to define a
function. Let's create a function that calculates the normal
distribution.

::

    double normal(double x) {
      return exp(-x*x)/sqrt(2.0*M_PI);
    }

To register this function with the parser we need to add a single line
to the code. This line is added at the location of the comment after the
call to ``registerCMath()``.

::

    P.getFunctionRegistry().registerFunction("normal",normal);

The string ``"normal"`` specifies the name of the function in the setup
file. This can be anything you like. The second argument is the function
in C++ itself. In fact, without the parentheses, ``normal`` specifies
the function pointer. **Note:**\ This needs to be a proper function and
can't be a class member function or a function object. This is all there
is to it. **Warning!** There is one caveat when using user defined
functions. The Schnek routines for evaluating expressions are
*intelligent*. When using a ``DependencyUpdater`` multiple times to
evaluate an expression with varying independent parameters, they
optimise the evaluation to minimise repeated evaluation of constant
expressions. While doing this they assume that functions are
deterministic, i.e. that a function will always return the same value
when given the same arguments. This poses a problem when adding
functions such as random number generators. The random number generator
will be called once and the result will be re-used for all future
evaluations of the expression. To avoid this problem, these type of
functions should be made depenent on a dummy parameter so that they are
not optimised away. The code for this example can be found
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_setup_functions.cpp>`__.
