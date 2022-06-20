.. _overview-label:

What is GaspiCxx?
=================

GaspiCxx is an open-source, high-level C++ abstraction layer built on top of
the `GPI-2 <https://github.com/cc-hpc-itwm/GPI-2>`_ communication library.

GaspiCxx provides the following features:

* Automatic resource management (memory, groups of processes)
* One-sided point-to-point communication primitives
* Blocking and non-blocking collective communication
* Encapsulation of GPI-2 concepts - segments, queues, groups, or allocations within the partitioned global address space


GaspiCxx - The Productive Approach to Efficient Parallel Programming
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The GaspiCxx interface is designed as an abstraction for the native C based communication
interface of GPI-2 that enhances productivity without an impact on the underlying performance.

The interface design makes the explicit management of communication
resources required by the native C interface fully transparent to the application.
GaspiCxx provides objects with exclusive and automatically managed resources
for groups, queues, segments. The dynamic management of segment
memory and segment synchronization primitives follow the allocator concept.
Efficient single-sided communication primitives, as well as collective primitives are built
on top of that.

GaspiCxx makes the use of GPI-2 productive and efficient, both enabling rapid development of
new applications, as well as easing the porting of existing ones.

PyGPI for Python-based Communication
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

GaspiCxx provides a Python extension called PyGPI, which exposes the GaspiCxx point-to-point
and collective primitives as an easy-to-use, intuitive Python library.