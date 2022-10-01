This short tutorial will show you how to handle and report errors from
the parser. Schnek uses the C++ exception handling system to process
errors. This makes it very easy to report syntax errors and undefined
variables. In the previous tutorial we have seen how to read values from
a configuration file. But what happens if there is a syntax error in the
configuration file? Let's modify the file and deliberately introduce an
error into the file.

::

    size = 123;
    dx = 3.056 + ;
    name = "Setup Test";

In the second line of the input file I have added a plus symbol,
creating an expression that can't be interpreted properly. If we run
this file through the example code from the previous tutorial the output
will look something like this.

::

    terminate called after throwing an instance of 'schnek::ParserError'
    Aborted (core dumped)

This is not very informative, but as you can see a ``ParserError`` has
been thrown. We can catch that error and extract a bit more information
from it. The following code shows how this is done.

::

      pBlock application;

      try
      {
        application = P.parse(in,"File Name");
      }
      catch (ParserError &e)
      {
        std::cerr << "Parse error in " << e.getFilename() << " at line " << e.getLine() << ": "<< e.message << "\n";
        exit(-1);
      }

The ``ParserError`` class contains three methods that provide
information, ``getFilename()`` returns a predefined name of the file.
This name can be specified as an optional second argument to the
``parse()`` method of the parser. The method ``getLine()`` returns the
line number on which the error occured and the method ``getMessage()``
returns a more detailed error message. With this modification of the
code we now get the much nicer error

::

    Parse error in File Name at line 2: Syntax Error

The ``ParserError`` class is thrown when the parser encounters syntax
errors or other errors that are related to the structure of the
configuration file. Another type of error is thrown when the evaluation
of a variable fails. Obviously, this error needs be caught when
evaluating the parameters.

::

      SimulationBlock &mysim = dynamic_cast<SimulationBlock&>(*application);
      try
      {
        mysim.evaluateParameters();
      }
      catch (EvaluationException &e)
      {
        std::cerr << "Evaluation error: " << e.getMessage() << std::endl;
      }

It turns out, however, that this is not enough. Schnek detects constant
expressions while parsing the configuration file and evaluates them
immediately. This means that an evaluation error can also be thrown by
the parser. I recommend surrounding the complete code for reading,
parsing and evaluating the configuration file with a single try — catch
block.

::

    try
      {
        application = P.parse(in,"File Name");

        SimulationBlock &mysim = dynamic_cast<SimulationBlock&>(*application);
        mysim.evaluateParameters();

        mysim.writeValues();
      }
      catch (ParserError &e)
      {
        std::cerr << "Parse error in " << e.getFilename() << " at line " << e.getLine() << ": "<< e.message << std::endl;
        exit(-1);
      }
      catch (EvaluationException &e)
      {
        std::cerr << "Evaluation Error: " << e.getMessage() << std::endl;
        exit(-1);
      }

Let's try this with the following input where a string is assigned to
the integer ``size`` variable.

::

    size = "nonumber";
    dx = 3.056;
    name = "Setup Test";

When running the code now we get a message that tells us that a type
conversion error occured during the evaluation.

::

    Evaluation Error: Could not convert types

**Side Note:** In the example above the string could not be converted to
an integer. The reason was that the content did not represent an integer
number. Schnek does perform automatic type conversion where possible. An
input file like the following is perfectly valid.

::

    string a = "12";
    string b = "34";

    size = a+b;

After this ``size`` will have a value of 1234. The code for this
tutorial can be found in the Schnek repository
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_setup_errors.cpp>`__.
