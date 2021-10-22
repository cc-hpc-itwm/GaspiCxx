
# PyGPI python communication library

PyGPI is a communication library designed to export GaspiCxx primitives to Python.
Its goal is to provide easy-to-use, intuitive access to GPI-based communication, focusing on the
following features:
* [X] Global access functions such as `get_rank`, `get_size`
* [X] `Group` functionality for defining processes involved in a particular communication
* [ ] Single-sided point-to-point communication using `Source/TargetBuffer`s
* Non-blocking collective communication
  - [X] Allreduce
  - [X] Broadcast
  - [ ] Allgather(v)
  - [ ] Gather(v)
* Blocking collective communication
  - [ ] Barrier


## Assumptions
* PyGPI is designed for communication of contiguous, homogeneous arrays
  * Supported types: `np.array`, Python `list`, Python scalars
* Default datatype is `np.float32`
  * The user can set the datatype when creating a communication object (e.g., Broadcast) by specifying a `dtype` argument
  * Supported dtypes might differ depending on the collective used
  * Typical dtypes: `int`, `float`, `np.int32`, `np.int64`, `np.double`
* Each collective is initialized with a default algorithm
  * Users can specify other algorithms with an `algorithm` argument to the constructor

## Limitations

* Returned result is **always** a contiguous `np.array`
* The number of dimesions of the input arrays is not preserved in the output; reshaping is needed
* `list` elements are assumed to be homogeneous (same type as specified in the `dtype` argument of the communication primitive)
* `strings` are not supported

## User interface

Each collective is an object built for a specific communication pattern defined by a **number of elements** of a given
**dtype**, which are arranged in a contiguous memory buffer (i.e. a `list` or Numpy `array`).
These parameters describe the only configuration allowed for input arrays (using other types of arrays results in
unspecified behaviour).

Each (non-blocking) collective provides two methods:
* `start` (with or without `input` parameters depending on collective): local, non-blocking method signaling 
the start of communication
* `wait_for_completion`: blocking function that returns the result of the operation (or `None` for the ranks that do
not require a result).

The collective object can be reused for multiple communications on different input buffers.

Example for `Allreduce`:

```
import pygpi

group_all = pygpi.Group()
dtype = long
input_array = np.ones(number_elements, dtype)
allreduce = pygpi.Allreduce(group_all, number elements, pygpi.ReductionOp.SUM,
                            dtype = dtype)
allreduce.start(input_array)
output_array = allreduce.wait_for_completion()
```

## TODOs
* add helper functions to list available algorithms/dtypes for each collective

