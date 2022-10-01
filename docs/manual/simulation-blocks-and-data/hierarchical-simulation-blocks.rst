In order for simulation codes to be flexible, they should allow the end
user to switch different physical effects on or off. They may also allow
the user to switch between different algorithms. In addition the user
may want to include additional calculations on demand, such as the time
averaging of fields. These calculations should only be performed when
needed and it should be possible to configure these choices through the
configuration file without having to re-compile the code. The
hierarchical block structure in Schnek allows the developer to create
modules of simulation code which can be included in the configuration
file. This allows for highly versatile codes. In Schnek, each type of
simulation block is represented by a class that descends from the
``Block`` class. The ``Block`` class was already introduced in the
`first example on setup
files <http://www.notjustphysics.com/schnek/schnek-documentation/reading-setup-files/a-first-example/>`__
as the class that represents the complete simulation. However a
simulation code can have a large number of different block types and
these can be arranged in a hierarchical manner. To start with a simple
example, let's assume we want to create a simulation with a number of
individual particles. Each particle can have its own mass, position and
velocity. In Schnek these particles could be represented by the
following code.

::

    class Particle : public Block {
      private:
        Array<double,3> pos;
        Array<double,3> velocity;
        double mass;

      protected:
        void initParameters(BlockParameters &parameters) {
          parameters.addArrayParameter("pos", pos);
          parameters.addArrayParameter("velocity", pos);
          parameters.addParameter("mass", &mass);
        }
      public:
        void advance(const Array<double,3> &force, double dt) {
          pos = pos+velocity*dt;
          velocity = velocity + force*dt/mass;
        }
        
        void display() {
          std::cout << "Mass: " << mass << std::endl;
          std::cout << "Position: (" << pos[0] << ", " << pos[1] << ", " << pos[1] << ")" << std::endl;
          std::cout << "Velocity: (" << velocity[0] << ", " << velocity[1] << ", " << velocity[1] << ")" << std::endl;
        }
    };

The ``Particle`` class is very much like the ``Block`` classes we have
seen in the `previous
chapter <http://www.notjustphysics.com/schnek/schnek-documentation/reading-setup-files/>`__.
The private members ``pos``, ``velocity``, and ``mass`` are registered
with the setup file parser in the ``initParameters()`` function. Two
additional functions, ``advance()`` and ``display()`` have been defined.
The first function will advance the particle position and velocity based
on an external force. The second function will print out the values to
the console. Very similar to the ``Particle`` class, we define a class
that will act as the root class for the simulation.

::

    class NBody : public Block {
      private:
        double dt;
        double totalTime;

      protected:
        void initParameters(BlockParameters &parameters) {
          parameters.addParameter("dt", &dt);
          parameters.addParameter("totalTime", &totalTime);
        }
        
      public:
        void runSimulation();
    };

Again, this class looks very much like the block classes we have seen
before. We have added an additional function ``runSimulation()`` that
will run the simulation. We will implement this function later. The
logical arrangement of these two classes is that an object of type
``NBody``, as the main simulation class, should contain an arbitrary
number of ``Particle`` objects. We can tell the setup file parser this
relationship in a few simple commands. In the ``main()`` function simply
include the following lines

::

      BlockClasses blocks;

      blocks.registerBlock("NBody").setClass<NBody>();
      blocks.registerBlock("Particle").setClass<Particle>();

      blocks("NBody").addChildren("Particle");

These lines will register two block types with the ``BlockClasses``
object. As introduced in the previous chapter, the string argument of
the ``registerBlock()`` function determines the name given to the class
in the setup file. The last line of the fragment above specifies the
relationship between the two classes. Here ``"Particle"`` is specified
as the child class of ``"NBody"``. The rest of the main function follows
the well known pattern.

::

    std::ifstream in("example_blocks_hierarchy01.setup");
    Parser P("nbody_simulation", "NBody", blocks);
    pBlock application = P.parse(in);

    NBody *mysim = dynamic_cast<NBody*>(application.get());
    mysim->evaluateParameters();
    mysim->runSimulation();

We set the ``NBody`` class as the main class of the simulation and then
parse the configuration file through an input file stream. Because we
know that the ``pBlock`` pointer really points to an ``NBody`` object we
can perform a cast. Then we first need to evaluate the parameters before
we finally run the simulation. After this we can write the setup file.
In the following example we include two particles.

::

    dt = 0.01;
    totalTime = 1;

    Particle A {
      posx = 0;
      posy = 0;
      posz = 0;
      velocityx = 0;
      velocityy = 0;
      velocityz = 0;
      mass = 10;
    }

    Particle B {
      posx = 1;
      posy = 0;
      posz = 0;
      velocityx = 0;
      velocityy = 10;
      velocityz = 0;
      mass = 1;
    }

When the code reads this file it will first create an ``NBody`` object
and set the ``dt`` and ``totalTime`` members. After this, two
``Particle`` objects are created, each with its own values for ``pos``,
``velocity``, and ``mass``. These two objects are added as children to
the ``NBody``. Inside the ``Particle`` class you can get the surrounding
``NBody`` object through the ``getParent()`` function. This function
returns a ``pBlock`` which is a boost shared pointer to the parent
block. Inside the ``NBody`` class you can get a collection of
``Particle`` objects through the ``getChildren()`` function. This will
return a list of ``pBlock`` pointers. You should use the ``BlockList``
type to store this list. This allows us to write the code for the
``runSimulation()`` function.

::

    void NBody::runSimulation() {
      BlockList children = getChildren();
      Array<double,3> force(0.0,0.0,0.0);
      for (double time=0.0; time<=totalTime; time+=dt) {
        BOOST_FOREACH(pBlock block, children) {
          boost::shared_ptr<Particle> particle = boost::dynamic_pointer_cast<Particle>(block);
          particle->advance(force, dt);
        }
      }
      BOOST_FOREACH(pBlock block, children) {
        std::cout << "Child: " << block->getName() << std::endl;
        boost::shared_ptr<Particle> particle = boost::dynamic_pointer_cast<Particle>(block);
        particle->display();
      }
    }

This function iterates through the time steps and for each time step it
loops over all children, advancing their positions. After the simulation
has finished another loop prints out the positions of all the particles.
Note the use of the ``BOOST_FOREACH`` macro here. If you are compiling
using the C++11 standard you can use a ranged based for loop
``for (pBlock block : children)`` instead. Please not however, that
Schnek has not yet been tested under the C++11 standard. We are using a
dynamic cast to cast the ``pBlock`` pointer to a shared pointer of
``Particle`` type. We can do this because we know that all children of
``NBody`` must be of type ``Particle``. This is OK for the example
presented here. In general we are allowed to declare different classes
as child classes of a block. In this case we would need to check for
each child what class it is and perform the required operation only on
those objects that match a given class. In the next section we will see
how this problem is solved by Schnek. The code for this example can be
downloaded
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_blocks_hierarchy.cpp>`__.
The setup file can be found
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_blocks_hierarchy01.setup>`__.
 
