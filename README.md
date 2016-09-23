# Schnek
## A C++ library for developing parallel simulations on Cartesian grids

<img align="left" src="http://www.notjustphysics.com/wp-content/uploads/2014/06/SchnekLogo.png" width="300">

Schnek is a new library for C++ that makes it easy to develop large parallel 
simulation codes. The library is intended mainly for use in physics simulations 
on regular grids but some of its features might be useful for other types of 
simulation codes.

When writing a new simulation code from scratch one is confronted with a choice. 
One can either keep the code simple and focus on the physics and the numerical 
side. For this kind of code, most time is spent on actually developing the 
interesting stuff. The downside of this type of code is that is not much use to 
anybody else except the developer of the code herself and maybe the immediate 
collaborators in a research group.

The other option is to develop a code that might be useful for others, that can 
be configured using a configuration file and that is written to be versatile and 
adaptable to problems other than the one it was initially developed for. The 
code should be easy to use by others and hopefully should also be extendable by 
others. Writing this type of code requires a lot of time and effort. Only very 
few projects get to a stage where this investment of time and work is 
worthwhile.

In addition, if the code handles large amounts of data and should be able to run 
on multi-processor clusters then additional code has to be written. For 
simulations on regular grids this means that one has to use multi-dimensional 
arrays, define boundary values, synchronise grid values accross multiple 
processes and write data into files that can be used by other software.

Schnek provides a single library that aids in all these tasks and thus massively 
reduces the code that needs to be written by the developer.

Features of Schnek are

- setup file reader with C style syntax
- extendable for formulas in setup files
- hierarchical structure of simulation components
- mutli-dimensional grids
- initialisation of grid from formulas in setup file
- boundary cells and ghost cells
- MPI support for parallelisation
- MPI update of ghost cells to neighbouring processes
- easy to use parallel and serial HDF5 output of grids
- literature reference system to encourage collaborative development of large 
  codes
  
## Documentation

Documentation and tutorials can be found on the Schnek website at

  http://www.notjustphysics.com/schnek/

 
## Installation

### Step 1: Download

You can obtain the latest release of Schnek on the GitHub Releases page, (https://github.com/holgerschmitz/Schnek/releases). This should be relatively bug-free. If you want to use Schnek for your own projects then this is your choice.

If you are interested in contributing and helping to improve Schnek, you are very welcome to do so. In this case you can grab the latest development version using Subversion. The command is

```
  git clone https://github.com/holgerschmitz/Schnek.git
```

### Step 2: Configuration

The configuration and installation script have been generated using the GNU automake utility. For the typical configuration simply type

```
  ./configure
```
  
in the directory that you unpacked Schnek into. This will run the configure script and create make files for the next step of the installation process.

For more control over the configuration you can pass options to configure. If you want to install Schnek into a directory different from the standard installation directory, use the --prefix option, like this

```
  ./configure --prefix=/your/installation/directory/
```

Schnek can be compiled with or without MPI support. By default Schnek will be compiled with MPI support if MPI can be found. You can force MPI to be enabled or disabled using the --with-mpi option. The following command will disable MPI even on those systems where it is present.

```
  ./configure --with-mpi=no
```

If MPI is enabled, Schnek will be compiled using the mpic++ compiler command. If you want to control mpic++ you should set your environment variables according to the documentation of your MPI version.

You can switch HDF5 support on or off using the --with-hdf5 option. You can specify yes or no to enable or disable HDF5. By default, HDF5 support is enabled. If your HDF5 headers are located in a non-standard location you can also specify the path to your HDF5 installation.

```
  ./configure --with-hdf5=/path/to/hdf5/
```

A full list of available options written out with the --help option.

```
  ./configure --help
```

### Step 3: Building the library

Once you have configured Schnek, type

```
  make
```

to build the library.

### Step 4: Installing

Finally, install the library by typing

```
  make install
```

or

```
  sudo make install
```

depending on the installation directory.

### Step 5: Include Schnek in your projects

By default the Schnek headers are located in /usr/local/include/schnek/. To include the headers in your C++ files, you should include them like this.

```
  #include <schnek/grid.hpp>
```

You nee to link your code to the Schnek libraries. For most compilers this should be achieved by adding -lschnek to your linker flags.

## License and Disclaimer

Schnek is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Schnek is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Schnek.  If not, see <http://www.gnu.org/licenses/>.
