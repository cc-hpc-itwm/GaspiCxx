.. _faq-label:

Frequently Asked Questions (FAQ)
================================

This is a list of frequently asked questions about GaspiCxx.
Please feel free to :ref:`suggest new ones <contact-label>`!

.. admonition:: Question

   How can I ssh to ``localhost`` without password?

In order to run GaspiCxx programs, you will need to be able to ssh to ``localhost`` without password.
First, generate an ``ssh`` key-pair:

.. code-block:: bash

   cd ~/.ssh
   ssh-keygen

Make sure not to overwrite existing keys.
When asked for a passphrase, ``Enter passphrase (empty for no passphrase):``, simply leave empty
and return with enter.
Also take specific care to set correct user rights on all files in ``.ssh``,
cf. for instance `here <https://superuser.com/questions/215504/permissions-on-private-key-in-ssh-folder>`__.
Next, append the public key to the ``authorized_keys`` file:

.. code-block:: bash

   cat id_rsa.pub >> authorized_keys

Install and start an ``ssh`` server, e.g., ``openssh-server`` on Fedora.
More details can be found for instance
`here <https://linuxconfig.org/how-to-install-start-and-connect-to-ssh-server-on-fedora-linux>`__.

.. admonition:: Question

   I get an execution error ``GPI library initialization incorrect environment vars`` when
   trying to run my script. What shall I do?

Most likely you are running your program with ``./my_executable``.
Please make sure to execute your code with ``gaspicxx_run -- ./my_executable`` instead.

.. admonition:: Question

   I get an execution error ``GPI library initialization general error``. What shall I do?

This error occurs when the underlying GPI-2 library tries to connect to a previously used
socket, which is not yet released.
Try to re-run your code after a short while so that the port becomes available again.

.. admonition:: Question

   The execution seems to stall. What shall I do?

Please use the ``gaspicxx_run --cleanup`` command to kill any processes that
might be still running from a previous (aborted) call to ``gaspicxx_run`` as shown
:ref:`here <gaspicxx-run-cleanup-label>`.
Note that you can also interrupt a running GaspiCxx instance (distributed on multiple nodes)
by using ``Ctrl+c``.

.. admonition:: Question

   | When trying to build GaspiCxx, CMake cannot find pybind11:
   | ``Could not find a package configuration file provided by "pybind11" with any``
   | ``of the following names: [...]``
   | What shall I do?

This error occurs when pybind11 is installed using pip.
Please use conda instead, as recommended in the :ref:`installation guide <python-deps-install-label>`.

.. admonition:: Question

   When trying to build GaspiCxx, CMake does not detect the Python interpreter from the
   active conda environment. What shall I do?

You will need to manually add the path to the conda environment's ``bin`` directory to your ``PATH``.
You will also need to specify the path to the python library on the command line when configuring GaspiCxx:

.. code-block:: bash

   PATH_TO_CONDA_ENV=/path/to/conda/env
   export PATH=${PATH_TO_CONDA_ENV}/bin:${PATH}
   cmake -DPYTHON_EXECUTABLE=${PATH_TO_CONDA_ENV}/bin/python \
         -DPYTHON_LIBRARY=${PATH_TO_CONDA_ENV}/lib ../


.. admonition:: Question

    Can I install GaspiCxx on MacOS?

GaspiCxx is only supported on Linux systems, as its GPI-2 dependency is built on top of a
Linux kernel API called `epoll`.
