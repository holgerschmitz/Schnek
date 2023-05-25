Step 1: Download
~~~~~~~~~~~~~~~~

You can obtain the `latest release of Schnek on the GitHub repository,
here <https://github.com/holgerschmitz/Schnek/releases>`__. This should
be relatively bug-free. If you want to use Schnek for your own projects
then this is your choice. If you are interested in contributing and
helping to improve Schnek, you are very welcome to do so. In this case
you can grab the latest development version using **git**. The command
is

::

    git clone https://github.com/holgerschmitz/Schnek.git

You now have downloaded the master branch into a new directory called
Schnek. If you want to fix a bug in the li git checkout -b develop
origin/develop

The second command above switches to the develop branch. This branch
should be used as the basis when adding new features to the library.

Step 2: Configuration
~~~~~~~~~~~~~~~~~~~~~

The configuration and installation script have been generated using the
GNU automake utility. For the typical configuration simply type

::

    ./configure

in the directory that you unpacked Schnek into. This will run the
configure script and create make files for the next step of the
installation process. For more control over the configuration you can
pass options to configure. If you want to install Schnek into a
directory different from the standard installation directory, use the
--prefix option, like this

::

    ./configure --prefix=/your/installation/directory/

Schnek can be compiled with or without MPI support. By default Schnek
will be compiled with MPI support if MPI can be found. You can force MPI
to be enabled or disabled using the --with-mpi option. The following
command will disable MPI even on those systems where it is present.

::

    ./configure --with-mpi=no

If MPI is enabled, Schnek will be compiled using the mpic++ compiler
command. If you want to control mpic++ you should set your environment
variables according to the documentaion of your MPI version. You can
switch HDF5 support on or off using the --with-hdf5 option. You can
specify yes or no to enable or disable HDF5. By default, HDF5 support is
enabled. If your HDF5 headers are located in a non-standard location you
can also specify the path to your HDF5 installation.

::

    ./configure --with-hdf5=/path/to/hdf5/

A full list of available options written out with the --help option.

::

    ./configure --help

Step 3: Building the library
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Once you have configured Schnek, type

::

    make

to build the library.

Step 4: Installing
~~~~~~~~~~~~~~~~~~

Finally, install the library by typing

::

    make install

or

::

    sudo make install

depending on the installation directory.

Step 5: Include Schnek in your projects
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

By default the Schnek headers are located in /usr/local/include/schnek/.
To include the headers in your C++ files, you should include them like
this.

::

    #include <schnek/grid.hpp>

You nee to link your code to the Schnek libraries. For most compilers
this should be achieved by adding -lschnek to your linker flags.
