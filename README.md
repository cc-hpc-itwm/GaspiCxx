# GaspiCxx

GaspiCxx is a C++ interface for the GPI-2 communication library,
which aims at providing a high-level abstraction layer on top of
the native communication primitives.
GaspiCxx is designed to achieve the following goals:
* scalability
* performance
* productivity

The interface design replaces the explicit management of communication
resources required by the native GPI-2 interface with fully-transparent
and easy-to-use primitives.
The GaspiCxx API provides the following automatically managed resources:
* Groups
* Segments
* Queues
* Point-to-point single-sided communication primitives
  * SourceBuffer/TargetBuffers
* Non-blocking collective primitives
  * Allreduce
  * Allgatherv
  * Broadcast
* Blocking collectives
  * Barrier

## Installation

### Compiler and build system

GaspiCxx can be built using a recent compiler with support for C++17.
It was tested with [gcc](https://gcc.gnu.org/) starting from version `8.4.0` and [clang](https://clang.llvm.org/) `10.0.0`.
You will also need the build tool [CMake](https://cmake.org/) (from version `3.12`).


### Step 1: Software dependencies

GaspiCxx depends on:

- [GPI-2](https://github.com/cc-hpc-itwm/GPI-2)
- (Optional) Google test

#### Installing GPI-2

To install `GPI-2`, clone the following [git repository]
(https://github.com/cc-hpc-itwm/GPI-2.git)
and checkout the `1.5.0` tag:

```bash
git clone https://github.com/cc-hpc-itwm/GPI-2.git
cd GPI-2
git fetch --tags
git checkout -b v1.5.0 v1.5.0
```

Now, use [autotools](https://www.gnu.org/software/automake/)
to configure and compile the code:

```bash
./autogen.sh
export GPI2_INSTALLATION_PATH=/your/installation/path
CFLAGS="-fPIC" CPPFLAGS="-fPIC" ./configure --with-infiniband --prefix=${GPI2_INSTALLATION_PATH}
make $nprocs
```

where `${GPI2_INSTALLATION_PATH}` needs to be replaced with the path where you want to install
GPI-2. Note the `--with-infiniband` option, which should be used on most HPC clusters that
provide Infiniband support.

In case you want to install GPI-2 on a laptop or workstation, replace the above
option with ``--with-ethernet``, which will use standard TCP sockets for communication.
Now you are ready to install GPI-2 with:

```bash
make install
export PATH=${GPI2_INSTALLATION_PATH}/bin:$PATH
```

### Step 2: Build GaspiCxx

Compile and install the GaspiCxx library as follows:

```bash
git clone git@gitlab.itwm.fraunhofer.de:gruenewa/GaspiCxx.git
cd GaspiCxx
mkdir build && cd build

export GASPICXX_INSTALLATION_PATH=/your/gaspicxx/installation/path
cmake -DCMAKE_INSTALL_PREFIX=${GASPICXX_INSTALLATION_PATH} ../
make -j$nprocs install
```

### (Optional) Step 3: Run tests

#### Install Google Test

Download and install Google test from its
[GitHub repository](https://github.com/google/googletest/tree/main/googletest).

```bash
export GTEST_INSTALLATION_PATH=/path/to/gtest

git clone https://github.com/google/googletest.git -b release-1.11.0
cd googletest
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=${GTEST_INSTALLATION_PATH} ../
make -j$nprocs install
```

#### Compile GaspiCxx with testing enabled

```bash
export GASPICXX_INSTALLATION_PATH=/your/gaspicxx/installation/path
cmake -DENABLE_TESTS=ON -DCMAKE_PREFIX_PATH=${GTEST_INSTALLATION_PATH} -DCMAKE_INSTALL_PREFIX=${GASPICXX_INSTALLATION_PATH} ../
make -j$nprocs install
```

#### Run tests
GaspiCxx tests can be executed using the `ctest` command
```bash
# list tests
ctest -N

# run all tests
ctest
```


## License
This project is licensed under the GPLv3.0 License - see the 
[COPYING](COPYING) file for details
