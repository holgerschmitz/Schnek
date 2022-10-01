The ``Block`` class has a number of virtual functions that can be
implemented by a concrete block class. These functions are intended to
allow the simulation to be initialised in an orderly fashion. The
initialisation is subdivided into a number of steps. This allows one
class to initialise data in an earlier step and another class to access
that data in a subsequent step, knowing that it has already been
initialised. In the `previous
sections <../simulation-blocks-and-data.html>`__
we have already come across some of these methods. In this section a
complete reference is given.

::

    class Block {
      protected:
        virtual void initParameters(BlockParameters&) {}
        virtual void registerData() {}
        virtual void preInit() {}
        virtual void init() {}
        virtual void postInit() {}
    };

By default all of the initialisation methods are defined as empty
functions. They may be overridden but they don't have to be. In the
following a brief description of each function is given.

-  ``void initParameters(BlockParameters&)`` This function is called
   just after the block object is created but before the contents of the
   block is read from setup file. Here the member variable can be added
   to the ``BlockParameters`` object. This registers the member
   variables with the parser and allows automatic initialisation of the
   member variables from the setup file. For details on this function
   see the `section on reading setup
   files <../reading-setup-files/a-first-example.html>`__.
-  ``void registerData()`` In this function data that is intended to be
   shared among multiple block should be registered using the
   ``addData()`` function. Once the fields have been registered other
   blocks are able to access them via the ``retrieveData()`` function.
   See the `previous section on sharing data between
   blocks <sharing-data-between-blocks.html>`__
   for details.
-  ``void preInit()`` This function is called before the main ``init()``
   function and allows additional initialisation that should be carried
   out before the main initialisation.
-  ``init()``

This is the main initialisation routine. This is the place where most of
the set up should go.

-  ``void postInit()`` This function is called after the main ``init()``
   function and allows additional initialisation that should be carried
   out after the main initialisation.

When the simulation hierarchy is created by the parser through
``Parser::parse()``, each block instance is created the moment that it
is read from the setup file. At this point the ``initParameters()``
method is called for the newly created block. This enables the parser to
read the remainder of the block's specification from the setup file.
After the parser has created the hierarch, a call to ``initAll()`` on
the root block of the hierarchy will automatically iterate over all
blocks for each of the remaining initialisation functions. However,
before the ``registerData()`` function is called on each block,
``initAll()`` will ensure that ``evaluateParameters()`` is called on
each block in the hierarchy. After ``registerData()`` function is called
on each block, ``preInit()`` is called on each block in the hierarchy,
then ``init()``, and finally ``postInit()``. This means that, for
example, during execution of the ``init()`` function it is guaranteed
that ``preInit()`` has already been called for every block in the
hierarchy. Below is a flow diagram outlining the parsing and the
initialisation sequence. [caption id="attachment\_425"
align="alignright" width="807"]\ |Schnek Block Initialisation Flow
Chart| Block initialisation lifecycle. Left part shows the flow of the
Parser::parse() function. The right part shows the flow of the
Block::initAll() function.[/caption] In order to clarify the flow using
an example, let's look at the following setup file.

::

    Particle A { 
      Nested N1 { }
    }

    Force B { }

    Force C { }

    Particle D { 
      Nested N2 { }
    }

We assume here that the blocks ``Particle``, ``Force``, and ``Nested``
have been set up with the parser according to the structure found in
this file. The parsing and the initialisation sequence is then

::

    Calling Parser::parse()
      Root Block initParameters
      Particle A initParameters
      Nested N1 initParameters
      Force B initParameters
      Force C initParameters
      Particle D initParameters
      Nested N2 initParameters

    Calling initAll()
      Root Block registerData
      Particle A registerData
      Nested N1 registerData
      Force B registerData
      Force C registerData
      Particle D registerData
      Nested N2 registerData
      
      Root Block preInit
      Particle A preInit
      Nested N1 preInit
      Force B preInit
      Force C preInit
      Particle D preInit
      Nested N2 preInit
      
      Root Block init
      Particle A init
      Nested N1 init
      Force B init
      Force C init
      Particle D init
      Nested N2 init
      
      Root Block postInit
      Particle A postInit
      Nested N1 postInit
      Force B postInit
      Force C postInit
      Particle D postInit
      Nested N2 postInit

This example illustrates in which order the initialisation functions are
called. Notice how the each routine is called for each block in the
order they appear in the setup file. This ordering is guaranteed by
Schnek. The code for this example can be found
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_blocks_lifecycle.cpp>`__
and the setup file is located
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_blocks_lifecycle.setup>`__.

.. |Schnek Block Initialisation Flow Chart| image:: http://www.notjustphysics.com/wp-content/uploads/2016/12/SchnekBlockInitialisation-1.png
   :class: wp-image-425 size-full
   :width: 807px
   :height: 1391px
   :target: http://www.notjustphysics.com/wp-content/uploads/2016/12/SchnekBlockInitialisation-1.png
