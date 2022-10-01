Data output in Schnek is handled by ``Diagnostic`` classes. These
classes inherit from the ``Block`` class and thus can be controlled by
the simulation's setup file. The typical behaviour of ``Diagnostic``
classes is to write some data to files at specified intervals. In
general the ``Diagnostic`` classes do not have to be called directly as
this is handled by the ``DiagnosticManager``. DiagnosticManager is a
singleton class that defines a number of methods used to control
diagnostic output. It stores all instances of the ``Diagnostic`` class
and keeps track of the current time.

::

    class DiagnosticManager
    {
      public: 
        static const DiagnosticManager& instance();
        
        void setTimeCounter(int *timecounter);
        void setPhysicalTime(double *physicalTime);
        
        void execute();

        double adjustDeltaT(double deltaT);
        
        // the following functions are not usually called directly
        void addIntervalDiagnostic(IntervalDiagnostic*);
        void addDeltaTimeDiagnostic(DeltaTimeDiagnostic*);
    };

Being a singleton you can not create instances of the
``DiagnosticManager`` directly. Instead you need to access the singleton
instance through the static ``instance()`` method. The ``Diagnostic``
class has two sub-classes that handle two different types of diagnotic.
``IntervalDiagnostic`` will write out data after a given number of
simulation steps, whereas ``DeltaTimeDiagnostic`` will write out data
after a specified physical time. The functions
``addIntervalDiagnostic()`` and ``addDeltaTimeDiagnostic()`` are used to
add diagnostic instances to the manager. These functions are not usually
called directly. Every instance of the ``IntervalDiagnostic`` or
``DeltaTimeDiagnostic`` will automatically call the relevant function to
add itself to the ``DiagnosticManager``. The simulation code should,
however, supply a reference to either a time counter or a physical time
through the functions ``setTimeCounter()`` and ``setPhysicalTime()``.
The argument is a pointer to the global simulation time step or the
global physical simulation time. The pointer must be valid throughout
the duration of the simulation. Usually these functions should be called
once during the startup of the simulation. Note that only one of the two
functions must be set if the simulation defines only
``IntervalDiagnostic`` or only ``DeltaTimeDiagnostic`` classes. On the
other hand it is allowed to mix the two types in a single simulation.
While the simulation is running the global time step counter or the
physical simulation time (or both) should be during after each
simulation step. After carrying out the numerical simulation step, a
call to ``execute()`` will iterate through all registered ``Diagnostic``
output objects and check if any data needs to be written. A skeleton of
a simulation loop is outlined below.

::

    int timeStep = 0;
    double time = 0.0;

    void run() {
      DiagnosticManager::instance().setTimeCounter(&timeStep);
      DiagnosticManager::instance().setPhysicalTime(&time);
      
      while (time <= simulationEndTime) {
        doSimulationStep();
        
        ++timeStep;
        time += dt;
        
        DiagnosticManager::instance().execute();
      }
    }

Another function that can be useful when using the physical simulation
time is ``adjustDeltaT()``. This function is intended to be called
before the numerical simulation step. Consider the case in which the
natural simulation time step ``dt`` is 0.2 but one of the diagnostic
outputs is requested through the setup file after a physical time
interval of 0.5. When the simulation time is 0.4 the next time step
would take it to 0.6, way beyond the desired output time of 0.5. The
``adjustDeltaT()`` calculates the next simulation time step, based on
the base time step and the time of the next diagnotic outputs. The code
skeleton below illustrates how it can be used.

::

    int timeStep = 0;
    double time = 0.0;
    double dt = 0.2;

    void run() {
      DiagnosticManager::instance().setTimeCounter(&timeStep);
      DiagnosticManager::instance().setPhysicalTime(&time);
      
      while (time <= simulationEndTime) {
        double dtAdjust = DiagnosticManager::instance().adjustDeltaT(dt);
        
        doSimulationStep(dtAdjust);
        
        ++timeStep;
        time += dtAdjust;
        
        DiagnosticManager::instance().execute();
      }
    }

A **serious warning** comes with this feature. Many numerical schemes
use staggered time steps, such as leap-frog schemes. Changing the time
step during the simulation can reduce the accuracy of the numerical
scheme drastically. It is therefore advised to only use the
``adjustDeltaT()`` function if you **know** that your numerical scheme
is not affected by this.
