
# PyGPI Python Communication Library

PyGPI is a communication library designed to provide easy-to-use, intuitive access to GPI-based
communication.
To this end, PyGPI makes GaspiCxx primitives available in Python, focusing on the
following features:
* [X] Global access functions such as `get_rank`, `get_size`
* [X] `Group` functionality for defining processes involved in a particular communication
* [ ] Single-sided point-to-point communication using `Source/TargetBuffer`s
* [ ] Non-blocking collective communication
  - [X] Allreduce
  - [X] Broadcast
  - [X] Allgatherv
  - [ ] Allgather
  - [ ] Gather(v)
* [x] Blocking collective communication
  - [x] Barrier


## Assumptions
* PyGPI is designed for communication of contiguous, homogeneous arrays
  * Supported types: `np.array`, Python `list`, Python scalars
* Default datatype is `np.float32`
  * The user can set the datatype when creating a communication object (e.g., Broadcast) by
    specifying a `dtype` argument
  * Supported dtypes might differ depending on the collective used
  * Typical dtypes: `int`, `float`, `np.int32`, `np.int64`, `np.double`
* Each collective is initialized with a default algorithm
  * Users can specify other algorithms with an `algorithm` argument to the constructor

## Limitations

* Returned result is **always** a contiguous `np.array`
* The number of dimensions of the input arrays is not preserved in the output; reshaping is needed
* `list` elements are assumed to be homogeneous (same type as specified in the `dtype` argument of the
  communication primitive)
* `strings` are not supported

## User interface

Each collective is an object built for a specific communication pattern defined by a
**number of elements** of a given **dtype**, which are arranged in a contiguous memory
buffer (i.e. a `list` or Numpy `array`).
These parameters describe the only configuration allowed for input arrays (using other types
of arrays results in unspecified behaviour).

Each (non-blocking) collective provides two methods:
* `start` (with or without `input` parameters depending on collective): local, non-blocking
method signaling the start of communication
* `wait_for_completion`: blocking function that returns the result of the operation
(or `None` for the ranks that do not require a result).

Additionally, each collective implements the following properties:
* `obj.collective` - the collective type, e.g. "Allreduce"
* `obj.algorithm` - which algorithm is in use
* `obj.dtype` - data type required for each of the buffer elements


The collective object can be reused for multiple communications on different input buffers.

Example for `Allreduce`:

```
import pygpi

group_all = pygpi.Group()
dtype = "long"
number_elements = 30
input_array = np.ones(number_elements, dtype)

allreduce = pygpi.Allreduce(group_all, number_elements, pygpi.ReductionOp.SUM,
                            dtype = dtype)
allreduce.start(input_array)
output_array = allreduce.wait_for_completion()

print(f"[rank {pygpi.get_rank()}] result = {output_array}")
```

## Quickstart

### Environment setup

Make sure a `python` interpretor is available on your machine, as well as the `pybind11` library.
This can be achieved for instance by creating a `conda` environment:
```bash
conda create -n pygpi
conda activate pygpi
conda install python=3.8

conda install pybind11 -c conda-forge
```

Note that [Pybind11](https://github.com/pybind/pybind11) is available through `pip` and `conda`.
However, the `pip`-package does not seem to include a cmake package. This is why we recommend
installing Pybind11 via `conda`.


### Install GaspiCxx with PyGPI support

Follow the installation steps for the dependencies of GaspiCxx in the [README](../../README.md) file.
Then compile and install GaspiCxx from the git repository as a shared library.

```bash
git clone https://github.com/cc-hpc-itwm/GaspiCxx.git
cd GaspiCxx
mkdir build && cd build

export GASPICXX_INSTALLATION_PATH=/your/gaspicxx/installation/path
cmake -DBUILD_PYTHON_BINDING=ON                              \
      -DBUILD_SHARED_LIBS=ON                                 \
      -DCMAKE_PREFIX_PATH=${GTEST_INSTALLATION_PATH}         \
      -DCMAKE_INSTALL_PREFIX=${GASPICXX_INSTALLATION_PATH} ../
make -j$nprocs install
```


### (Optional) Compile GaspiCxx tests
The `PyGPI` tests rely on the `pytest` package, which can be installed in the same `conda` environment using `pip`:
```bash
conda activate pygpi
pip install -U pytest
```

Then compile GaspiCxx with testing enabled:

```bash
export GASPICXX_INSTALLATION_PATH=/your/gaspicxx/installation/path
cmake -DENABLE_TESTS=ON                                      \
      -DBUILD_PYTHON_BINDINGS=ON                             \
      -DBUILD_SHARED_LIBS=ON                                 \
      -DCMAKE_PREFIX_PATH=${GTEST_INSTALLATION_PATH}         \
      -DCMAKE_INSTALL_PREFIX=${GASPICXX_INSTALLATION_PATH} ../
make -j$nprocs install
```


### Execute PyGPI example

* Create a `nodesfile` with one machine name (i.e., hostname) per line for each of the parallel processes that will be involved in the data parallel DNN training.
```bash
$cat ./nodesfile
localhost
localhost
```

* Create a `script.sh` file to execute a simple DNN training run:
```bash
cat script.sh
export PYTHONPATH=${GASPICXX_INSTALLATION_PATH}/lib/:${GASPICXX_INSTALLATION_PATH}/lib
export LD_LIBRARY_PATH=${GASPICXX_INSTALLATION_PATH}/lib:${LD_LIBRARY_PATH}

python ${SRC_PATH}/simple_collective_example.py
```


* Execute the script on all processes in parallel using:
```bash
gaspi_run -m ./nodesfile -n ${NUM_PROCESSES} ./script.sh
```
The `${NUM_PROCESSES}` variable should match the number of lines in the `nodesfile`.
The same `hostname` can be repeated multiple times in the `nodesfile` if multiple ranks per host are needed.

## TODOs
* add helper functions to list available algorithms/dtypes for each collective

