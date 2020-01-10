# GaspiCxx

GaspiCxx is a C++ interface for the communication library GPI-2. The C++ interface aims at an abstraction for the native C based communication interface of GPI-2 without impacting the underlying performance. The interface design makes the explicit management of communication resources required by the native 
C interface fully transparent to the application. They do not need to be managed explicitly any more. Instead, objects with exclusive and auto-managed resources for groups, queues, segments are provided by GaspiCxx. The dynamic management of segment memory and segment synchronization primitives is provided by allocators. The single-sided and the passive communication are built on top of that. Allocations within the partitioned global address space and the respective synchronization primitives can be easily connected with each other on the source and the target side for efficient communication. 

## Installation

### Step 1: Software dependencies

GaspiCxx depends on 

- Google test
- [GPI-2](https://github.com/cc-hpc-itwm/GPI-2)

### Step 2: Build GaspiCxx

CMake is used as build system The dependency to GPI-2 is resolved by pkg-config. In order to allow CMake to find the GPI-2 configuration files, the path `GPI2_INSTALL_DIR/lib64/pkgconfig` needs to be appended to the `PKG_CONFIG_PATH` environment variable before invocation of CMake, e.g. by

```
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:GPI2_INSTALL_DIR/lib64/pkgconfig
```

## License
This project is licensed under the GPLv3.0 License - see the 
[COPYING](COPYING) file for details
