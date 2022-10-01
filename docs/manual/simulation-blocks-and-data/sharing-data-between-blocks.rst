In many situations different parts of a simulation code have to access
the same bit of data. The traditional approach in many simulation codes
is to declare some global variables that every part of the code has
access to. The problem with this approach is that, apart from violating
the data encapsulation paradigm, it makes the simulation code
inflexible. The fields that are used are hard wired and any modification
will require modification and recompilation of the code.

Basics
~~~~~~

In Schnek data can be shared between simulation block in a dynamic way.
A simulation block can export references to internal data that it allows
other simulation blocks to access. The other block can obtain the
reference to the data and read its values or perform operations on it.
Let's look again at our example from the `previous
section <../handling-multiple-child-block-types.html>`__.
If we wanted to allow other blocks to have access to a ``Particle``'s
``pos`` and ``velocity`` field, we can add the ``registerData()`` method
to the ``Particle`` class.

::

    class Particle : public ChildBlock<Particle>
    {
      private:
        Array<double,3> pos;
        Array<double,3> velocity;
        double mass;

      protected:
        void registerData() {
          addData("pos", pos);
          addData("velocity", velocity);
        }
      
        ...

Inside the ``registerData()`` method, we are calling the ``addData()``
function which is available inside any ``Block`` class. This function
takes a string and a reference to a variable. The string acts as a
reference key by which other blocks can access the variable. In the
example above we are exporting ``pos`` and ``velocity`` under the names
``"pos"`` and ``"velocity"``. Note that you do not have to call the
``registerData()`` function ourselves. This function is automatically
called during the initialisation through the ``initAll()`` function of
the simulation block. In our example we want a second class to access
the particle data and write it to the console. We can create a new class
``WriteVector`` like this.

::

    class WriteVector : public ChildBlock<WriteVector> {
      private:
        Array<double,3> *pVector;

      protected:
        void init() {
          retrieveData("pos", pVector);
        }

      public:
        void display() {
          std::cout << (*pVector)[0] << " " << (*pVector)[1] << " " << (*pVector)[2] << std::endl;
        }
    };

    typedef boost::shared_ptr<WriteVector> pWriteVector;

The class contains a pointer to the ``Array`` data type that was
exported in the ``Particle`` class. By calling ``retrieveData()`` with
``"pos"`` as the reference key, this pointer is set to point to the
``pos`` member of the ``Particle`` block. Thus, whenever ``display()``
is called, the current value of the particle position is written to the
console. Again, the ``init()`` function does not need to be called
manually as it is automatically called during the initialisation of the
simulation block. In order to include the ``WriteVector`` class in our
simulation we need to make sure that the simulation block has access to
the children of type ``WriteVector``.

::

    class NBody : public Block, public BlockContainer<Particle>, public BlockContainer<Force>, public BlockContainer<WriteVector> {
        ...

In addition the ``WriteVector`` block has to be registered with the
parser.

::

    int main()
    {
      BlockClasses blocks;

      blocks.registerBlock("NBody").setClass<NBody>();
      blocks.registerBlock("Particle").setClass<Particle>();
      blocks.registerBlock("LinearForce").setClass<LinearForce>();
      blocks.registerBlock("NonLinearForce").setClass<NonLinearForce>();
      blocks.registerBlock("WriteVector").setClass<WriteVector>();

      blocks("NBody").addChildren("Particle")("LinearForce")("NonLinearForce")("WriteVector");
      
      ...

Now the ``runSimulation()`` function can write out the particle position
in each time step like this

::

    void NBody::runSimulation()
    {
      for (double time=0.0; time<=totalTime; time+=dt)
      {schnek/schnek-documentation/simulation-blocks-and-data/handling-multiple-child-block-types/
        BOOST_FOREACH(pParticle particle, BlockContainer::childBlocks())
        {
          Array<double,3> force(0.0,0.0,0.0);

          BOOST_FOREACH(pForce F, schnek::BlockContainer::childBlocks()) {
            force += F->getForce(particle->getPos());
          }

          particle->advance(force, dt);

          BOOST_FOREACH(pWriteVector write, BlockContainer::childBlocks()) {
            write->display();
          }
        }
      }
    }

This is all that need to be done at this point. The setup file can now
include the ``WriteVector`` block. The body of the block specification
stays empty because we haven't registered any setup parameters for the
block type.

::

    dt = 0.01;
    totalTime = 100;

    Particle A {
      posx = 0.0;
      posy = 0.2;
      posz = 0;
      velocityx = 0;
      velocityy = 0;
      velocityz = 0;
      mass = 10;
    }

    NonLinearForce F {
      centerx = 1;
      centery = 0;
      centerz = 1;
      k = 1;
      d = 0.1;
    }

    WriteVector {
    }

Adding Flexibility
~~~~~~~~~~~~~~~~~~

Up to now we have still hard coded the relation between ``WriteVector``
and ``Particle``. The ``WriteVector`` will always print out the ``pos``
member of the particle. We can easiliy add flexibility to the code by
allowing the user to choose which member should be written out. To this
end we add a reference key string to the ``WriteVector`` class.

::

    class WriteVector : public ChildBlock<WriteVector> {
      private:
        Array<double,3> *pVector;
        std::string value;
      protected:
        void initParameters(BlockParameters &parameters) {
          parameters.addParameter("value", &value);
        }

        void init() {
          retrieveData(value, pVector);
        }

        ...

As in previous tutorials, we are using ``initParameters()`` to register
the ``value`` string with the parser. Inside the ``init()`` function we
use the value provided in the setup file and look up the vector
accordingly. The ``WriteVector`` definition in the setup file can now be
modified like this.

::

    WriteVector {
      value = "velocity";
    }

We could have also written ``value = "pos"``. With this feature we have
added flexibility to our code. The user can now decide in the setup file
where the data should be taken from.

Resolving Conflicts
~~~~~~~~~~~~~~~~~~~

Until now in this section we have implicitely assumed that there is only
one ``Particle`` block in our simulation. But in the `previous
section <../handling-multiple-child-block-types.html>`__
we showed that the user is free to add as many ``Particle`` blocks in
the setup file as desired. So let's assume we have two particles such as
this.

::

    Particle A {
      posx = 0.0;
      posy = 0.2;
      posz = 0;
      velocityx = 0;
      velocityy = 0;
      velocityz = 0;
      mass = 10;
    }

    Particle B {
      posx = 1.0;
      posy = 0.2;
      posz = 0;
      velocityx = 0;
      velocityy = 0;
      velocityz = 0;
      mass = 5;
    }

The reference ``value = "velocity"`` is now abiguous. It is not clear if
the velocity of particle ``A`` or particle ``B`` should be written out.
Fortunately, Schnek provides a mechanism to resolve ambiguities using
the dot notation. By specifying the particle name in the reference
string we can uniquely reference the velocity of a specific particle

::

    WriteVector {
      value = "B.velocity";
    }

This notation will work for more deeply nested block hierarchies as
well. In this case one could write something like
``value = "Outer.Inner1.Inner2.velocity"``. The code for this example
can be found
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_blocks_storage.cpp>`__.
The setup file can be found
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_blocks_storage.setup>`__
