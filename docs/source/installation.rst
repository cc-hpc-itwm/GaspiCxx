.. _installation-label:

Installation
============

The GaspiCxx library is available on `GitHub <https://github.com/cc-hpc-itwm/gaspicxx>`_. 

GaspiCxx relies on the following dependencies:

- `GPI-2 <https://github.com/cc-hpc-itwm/GPI-2>`_
- recent `gcc <https://gcc.gnu.org/>`_  or `clang  <https://llvm.org/>`_ compiler
- CMake
- Python 3.x
- [Optional] pybind11
- [Optional] Pytest
- [Optional] Google test

A step-by-step installation guide is presented in the following sections.

Software dependencies
---------------------

Compiler and build system
^^^^^^^^^^^^^^^^^^^^^^^^^

GaspiCxx can be built using a recent `gcc <https://gcc.gnu.org/>`_ 
or `clang  <https://llvm.org/>`_ compiler with support for C++17.
You will also need `CMake <https://cmake.org/>`_ (from version ``3.12``)
and a recent `Python 3.x` installation.


.. _gpi2-install-label:

Installing GPI-2
^^^^^^^^^^^^^^^^

The main dependency of GaspiCxx is the GPI-2 library, a low-level API for high-performance,
asynchronous communication for large scale applications, based on the
`GASPI (Global Address Space Programming Interface) standard <http://www.gaspi.de>`_.

GPI-2 is open-source and can be downloaded from its GitHub repository.
The currently supported versions start at ``v1.5``.
To download the required version, clone the
`GPI-2 git repository <https://github.com/cc-hpc-itwm/GPI-2.git>`_
and checkout the latest ``tag`` as shown below:

.. code-block:: bash

  git clone https://github.com/cc-hpc-itwm/GPI-2.git
  cd GPI-2
  git fetch --tags
  git checkout -b v1.5.1 v1.5.1

Now, use `autotools <https://www.gnu.org/software/automake/>`_ to configure and compile the code
with position independent flags (``-fPIC``):

.. code-block:: bash

  ./autogen.sh 
  export GPI2_INSTALLATION_PATH=/your/gpi2/installation/path
  CFLAGS="-fPIC" CPPFLAGS="-fPIC" ./configure --with-infiniband --prefix=${GPI2_INSTALLATION_PATH}
  make -j$(nproc)

where ``${GPI2_INSTALLATION_PATH}`` needs to be replaced with the path where you want to install
GPI-2. Note the ``--with-infiniband`` option, which should be used on most HPC clusters that
provide Infiniband support.

In case you want to install GPI-2 on a laptop or workstation, replace the above
option with ``--with-ethernet``, which will use standard TCP sockets for communication.

Now you are ready to install GPI-2 and make the library visible to your system with:

.. code-block:: bash

  make install
  export PATH=${GPI2_INSTALLATION_PATH}/bin:$PATH


.. _python-deps-install-label:

[Optional] Installing Python dependencies
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Installing GaspiCxx correctly requires access to a Python installation, either system-wide
or at the user level.

You can safely skip this section if Python is already present in your system and you do not
want to install PyGPI, the GaspiCxx python bindings library.

To install the additional Python libraries, we recommend creating a
`conda <https://docs.conda.io/en/latest/>`_ environment,
which allows for a self-contained installation, and ensures the same Python version is
bound to all dependencies.

In order to do that, first install `conda <https://docs.conda.io/en/latest/>`_ on your system.
Then, create and activate an environment for GaspiCxx:

.. code-block:: bash

  conda create -n gaspicxx
  conda activate gaspicxx
  conda install python

You can now install the remaining libraries 
(`pybind11 <https://pybind11.readthedocs.io/en/stable/index.html>`_
and `pytest <https://docs.pytest.org/>`_) within the `gaspicxx` environment.

.. code-block:: bash

  conda activate gaspicxx
  conda install pip
  conda install pybind11 -c conda-forge
  pip install -U pytest


.. _gaspicxx-install-label:

Installing GaspiCxx
-------------------

Now you can download the latest verson of the GaspiCxx library as follows:

.. code-block:: bash

  git clone https://github.com/cc-hpc-itwm/GaspiCxx.git
  cd GaspiCxx
  git fetch --tags
  git checkout -b v1.1.0 v1.1.0

You are now ready to install GaspiCxx!

.. code-block:: bash

  mkdir build && cd build

  export GASPICXX_INSTALLATION_PATH=/your/gaspicxx/installation/path
  cmake -DCMAKE_INSTALL_PREFIX=${GASPICXX_INSTALLATION_PATH} ../
  make -j$(nproc) install

Make sure the library is exported to the current ``PATH``. You should be able to execute
the ``gaspicxx_run`` command to test the installation:

.. code-block:: bash

  export PATH=${GASPICXX_INSTALLATION_PATH}/bin:${PATH}
  gaspicxx_run --help



[Optional] Installing Python bindings - PyGPI
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To install PyGPI, make sure you have created a ``gaspicxx`` conda environment which contains
the ``pybind11`` package as shown :ref:`here <python-deps-install-label>`.

.. code-block:: bash

  conda activate gaspicxx

  export GASPICXX_INSTALLATION_PATH=/your/gaspicxx/installation/path
  cmake -DBUILD_PYTHON_BINDINGS=ON    \
        -DBUILD_SHARED_LIBS=ON        \
        -DCMAKE_INSTALL_PREFIX=${GASPICXX_INSTALLATION_PATH} ../
  make -j$(nproc) install


[Optional] Compile tests
^^^^^^^^^^^^^^^^^^^^^^^^

Download and install Google test from its
`GitHub repository <https://github.com/google/googletest/tree/main/googletest>`_.

.. code-block:: bash

  export GTEST_INSTALLATION_PATH=/path/to/gtest

  git clone https://github.com/google/googletest.git -b release-1.11.0
  cd googletest
  mkdir build && cd build
  cmake -DCMAKE_INSTALL_PREFIX=${GTEST_INSTALLATION_PATH} ../
  make -j$(nproc) install


Then compile GaspiCxx with testing enabled

.. code-block:: bash

  export GASPICXX_INSTALLATION_PATH=/your/gaspicxx/installation/path
  cmake -DENABLE_TESTS=ON                                      \
        -DCMAKE_PREFIX_PATH=${GTEST_INSTALLATION_PATH}         \
        -DCMAKE_INSTALL_PREFIX=${GASPICXX_INSTALLATION_PATH} ../
  make -j$(nproc) install


GaspiCxx tests can be executed using the ``ctest`` command

.. code-block:: bash

  # list tests
  ctest -N

  # run all tests
  ctest


[Optional] Building documentation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you would like to build `the documentation <https://gaspicxx.readthedocs.io/en/latest/>`_
locally, make sure you have `Sphinx <https://www.sphinx-doc.org/en/master/>`_ installed
(potentially in the ``gaspicxx`` conda environment):

.. code-block:: bash

  conda activate gaspicxx
  pip install -U sphinx


Now re-compile GaspiCxx as follows

.. code-block:: bash

  export GASPICXX_INSTALLATION_PATH=/your/gaspicxx/installation/path
  cmake -DBUILD_DOCS=ON                                      \
        -DCMAKE_PREFIX_PATH=${GTEST_INSTALLATION_PATH}         \
        -DCMAKE_INSTALL_PREFIX=${GASPICXX_INSTALLATION_PATH} ../
  make -j$(nproc) install

The compiled documentation will be available in ``${GASPICXX_INSTALLATION_PATH}/docs``.


Passwordless SSH authentication
-------------------------------

In order to use GaspiCxx on a cluster, make sure you can ``ssh`` between nodes
without password. For details, refer to the :ref:`FAQ section <faq-label>`.
In particular, to test GaspiCxx on your local machine, make sure
you can ssh to ``localhost`` without password.

