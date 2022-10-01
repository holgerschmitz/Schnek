In many situations a ``Block`` might have many different child blocks.
The ``getChildren()`` function introduced in the `previous
section <http://www.notjustphysics.com/schnek/schnek-documentation/simulation-blocks-and-data/hierarchical-simulation-blocks/>`__
will always return all child blocks, independent of their type. To get
only blocks of certain types one would have to iterate through all child
blocks and use a dynamic cast to test for the different types.
Fortunately Schnek provides a mechanism to simplify this task. The two
class templates ``BlockContainer`` and ``ChildBlock`` supply the
framework for this mechanism. Let's extend the example from the
`previous
section <http://www.notjustphysics.com/schnek/schnek-documentation/simulation-blocks-and-data/hierarchical-simulation-blocks/>`__.
There we introduced a ``Particle`` block that served as a child block to
the ``NBody`` simulation block. We used the ``getChildren()`` function
to obtain a list of all child blocks. This list contained members of
type ``pBlock`` which is equivalent to ``boost::shared_ptr<Block>``.
Let's now explicitely declare that ``Particle`` blocks should be treated
child blocks of ``NBody``. We do this by first making sure that
``NBody`` inherits from the ``BlockContainer`` class.

::

    class NBody : public Block, public BlockContainer<Particle> {
      ...
    };

The second step is changing the inheritance of the ``Particle`` block
from the ``Block`` class to the ``ChildBlock`` class.

::

    class Particle : public ChildBlock<Particle> {
      ...
    };

    typedef boost::shared_ptr pParticle;

For convenience we have also added a typedef to a shared pointer of the
``Particle`` type. One more change is needed to make sure that
everything gets initialised properly. We need to change the call to
``evaluateParameters()`` in the ``main()`` function to a call to
``initAll()``.

::

    mysim->initAll();

``initAll()`` will call ``evaluateParameters()`` but will also call a
number of other functions to initialise the block hierarchy. This will
be detailed in the following section. With these modifications in place,
we are now able to rewrite the ``runSimulation()`` function.

::

    void NBody::runSimulation() {
      Array<double,3> force(0.0,0.0,0.0);
      for (double time=0.0; time<=totalTime; time+=dt) {
        BOOST_FOREACH(pParticle particle, childBlocks()) {
          particle->advance(force, dt);
        }
      }
      BOOST_FOREACH(pParticle particle, childBlocks()) {
        std::cout << "Child: " << particle->getName() << std::endl;
        particle->display();
      }
    }

The ``BlockContainer`` class template provides a member function
``childBlocks()``. This function returns an iterator range over shared
pointers to the template parameter. This means that
``BlockContainer<Particle>::childBlocks()`` will iterate over
``pParticle`` pointers. Notice how we have eliminated the type cast from
``pBlock`` to ``pParticle``. This is all well and good, but how does
this help us in our goal to have multiple types of child blocks? It
turns out that it allows us to create another type of ``ChildBlock``. If
you wanted to add a ``Force`` class that returns the force on a particle
depending on its position, then you could create something like this.

::

    class Force : public ChildBlock<Force> {
      private:
        Array<double,3> center;
        double k;
      protected:
        void initParameters(BlockParameters &parameters) {
          parameters.addArrayParameter("center", center);
          parameters.addParameter("k", &k);
        }
      public:
        Array<double,3> getForce(const Array<double,3> &pos) {
          return k*(center-pos);
        }
    };

    typedef boost::shared_ptr<Force> pForce;

This is a simple linear force that will implement a harmonic oscillator.
The ``NBody`` class needs to know that it should act as a container for
this class, so we add another inheritance to it.

::

    class NBody : public Block, public BlockContainer<Particle>, public BlockContainer<Force> {
      ...
    };

Now we have introduced a small problem. Both
``BlockContainer<Particle>`` and ``BlockContainer<Force>`` define a
``childBlocks()`` member function. Fortunately C++ allows us to resolve
the conflict by explicitely stating which function we want to call. The
``runSimulation()`` function can now be modified as follows.

::

    void NBody::runSimulation() {
      for (double time=0.0; time<=totalTime; time+=dt) {
        BOOST_FOREACH(pParticle particle, BlockContainer<Particle>::childBlocks()) {
          Array<double,3> force(0.0,0.0,0.0);

          BOOST_FOREACH(pForce F, schnek::BlockContainer<Force>::childBlocks()) {
            force += F->getForce(particle->getPos());
          }

          particle->advance(force, dt);
        }
      }
      BOOST_FOREACH(pParticle particle, BlockContainer<Particle>::childBlocks()) {
        std::cout << "Child: " << particle->getName() << std::endl;
        particle->display();
      }
    }

As before ``BlockContainer<Particle>::childBlocks()`` returns an
iterator range over ``pParticle`` pointers, while
``schnek::BlockContainer&lt;Force&gt;::childBlocks()`` now returns an
iterator range over ``pForce`` pointers. All we now need to do is to
register the ``Force`` class with the setup file parser.

::

    blocks.registerBlock("NBody").setClass<NBody>();
    blocks.registerBlock("Particle").setClass<Particle>();
    blocks.registerBlock("Force").setClass<Force>();

    blocks("NBody").addChildren("Particle")("Force");

Here you can also see how the ``addChildren()`` function can be chained
to add multiple child block types in one go. This will now allow us to
write a setup file such as the following, containing a particle and a
force.

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

    Force force {
      centerx = 1;
      centery = 0;
      centerz = 0;
      k = 0.5;
    }

We might decide that all this is not enough. We want multiple different
types of forces that use different formulae to calculate the force on a
particle. We can do this by using polymorphism. Instead of defining just
a single ``Force`` class, let's say we want to have a ``LinearForce``
and a ``NonLinearForce``. However, in the loop over all the forces, we
don't want to distinguish between the two. We can do this by creating a
type hierarchy.

::

    class Force : public ChildBlock<Force> {
      public:
        virtual ~Force() {};
        virtual Array<double,3> getForce(const Array<double,3> &pos) = 0;
    };

    typedef boost::shared_ptr<Force> pForce;

    class LinearForce : public Force {
      private:
        Array<double,3> center;
        double k;
      protected:
        void initParameters(BlockParameters &parameters) {
          parameters.addArrayParameter("center", center);
          parameters.addParameter("k", &k);
        }

      public:
        Array<double,3> getForce(const Array<double,3> &pos) {
          return k*(center-pos);
        }
    };

    class NonLinearForce : public Force {
      private:
        Array<double,3> center;
        double k;
        double d;
      protected:
        void initParameters(BlockParameters &parameters) {
          parameters.addArrayParameter("center", center);
          parameters.addParameter("k", &k);
          parameters.addParameter("d", &d);
        }

      public:
        Array<double,3> getForce(const Array<double,3> &pos) {
          Array<double,3> delta = center-pos;

          return k*delta / (delta.sqr() + d*d);
        }
    };

The ``Force`` class is now an abstract base class for our hierarchy. It
still inherits from ``ChildBlock<Force>`` so that the ``NBody`` class
can access all children of this type. The two classes ``LinearForce``
and a ``NonLinearForce`` simply inherit from ``Force`` and implement the
abstract ``getForce()`` method. Again, we can register these classes
with the setup file parser.

::

    blocks.registerBlock("NBody").setClass<NBody>();
    blocks.registerBlock("Particle").setClass<Particle>();
    blocks.registerBlock("LinearForce").setClass<LinearForce>();
    blocks.registerBlock("NonLinearForce").setClass<NonLinearForce>();

    blocks("NBody").addChildren("Particle")("LinearForce")("NonLinearForce");

The ``runSimulation()`` function does not need any modification. By the
virtue of polymorphism the correct ``getForce()`` function will be
called. A possible setup file could now look like this.

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

    LinearForce linear {
      centerx = 1;
      centery = 0;
      centerz = 0;
      k = 0.5;
    }

    NonLinearForce nonlinearA {
      centerx = 0;
      centery = 1;
      centerz = 0;
      k = 2;
      d = 0.1;
      
    }

    NonLinearForce nonlinearB {
      centerx = 1;
      centery = 0;
      centerz = 1;
      k = 1;
      d = 0.1;
    }

The code for this example can be downloaded
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_blocks_childblocks.cpp>`__
and the setup file can be found
`here <https://github.com/holgerschmitz/Schnek/blob/master/examples/example_blocks_childblocks.setup>`__.
