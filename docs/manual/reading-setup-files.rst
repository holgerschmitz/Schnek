One of the main features of Schnek is the ability to read and parse
setup files and initialise simulation parameters from the values
specified in the setup file. Schnek is quite versatile. The setup file
can contain mathematical expressions and the user can even define her
own variables inside the setup file to use inside the expressions. You
can initialise scalar variables of the standard types ``int``,
``double``, and ``std::string``. It is also possible to fill ``Grid``\ s
from the expressions supplied in the input file. You can allow the user
to specify a time-varying function that specifies a parameter that
varies during the course of the simulation run. Finally, it is easy to
extend the reader to accept additional functions. This section covers
the following topics.

-  `A first
   example <reading-setup-files/a-first-example.html>`__
-  `Error
   Handling <reading-setup-files/error-handling.html>`__
-  `Default Values, Arrays and
   Constants <reading-setup-files/default-values-arrays-and-constants.html>`__
-  `Evaluating
   expressions <reading-setup-files/evaluating-expressions.html>`__
-  `Filling fields <reading-setup-files/filling-fields.html>`__
-  `Adding user defined
   functions <reading-setup-files/adding-user-defined-functions.html>`__
