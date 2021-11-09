import pygpi_wrappers

import abc
import atexit
import inspect
import numpy as np

def list_implemented_primitives(module, primitive_name):
  classes = inspect.getmembers(module, inspect.isclass)
  return {name : cl for name, cl in classes if name.startswith(f"{primitive_name}_")}

def collective_class_selector(collective, dtype, algorithm):
  dtype = dtype_to_collective_type(dtype)
  implementations = list_implemented_primitives(pygpi_wrappers, collective)
  name = pygpi_wrappers.generate_implemented_primitive_name(collective, dtype, algorithm)
  if name not in implementations:
    raise ValueError(f"[CollectiveSelector] Unknown implementation `{name}` for collective {collective}")
  return implementations[name]

def dtype_to_collective_type(dtype):
  if dtype in [np.float,  np.float64, np.double, "double"]:
    return "double"
  elif dtype in [np.float32, "float"]:
    return "float"
  elif dtype in [np.int, np.int32, "int"]:
    return "int"
  elif dtype in [np.int16]:
    return "int16_t"
  elif dtype in [np.int64, "long"]:
    return "long"
  raise ValueError(f"[dtype_to_collective_type] Unknown dtype `{dtype}` for PyGPI primitives")


class Collective(abc.ABC):
  def __init__(self, *args, **kwargs):
    collective_class = collective_class_selector(self.collective, self.dtype,
                                                 self.algorithm)
    self.collective_impl = collective_class(*args, **kwargs)
    atexit.register(self.close)

  @property
  @abc.abstractmethod
  def collective(self):
    pass

  @property
  @abc.abstractmethod
  def dtype(self):
    pass

  @property
  @abc.abstractmethod
  def algorithm(self):
    pass

  def close(self):
    del self.collective_impl

  def start(self, *args, **kwargs):
    return self.collective_impl.start(*args, **kwargs)

  def wait_for_completion(self, *args, **kwargs):
    return self.collective_impl.wait_for_completion(*args, **kwargs)


class Allgatherv(Collective):
  def __init__(self, *args, algorithm = None, dtype = None, **kwargs):
    self._algorithm = algorithm or "ring"
    self._dtype = dtype or "float"
    super().__init__(*args, **kwargs)

  @property
  def collective(self):
    return "Allgatherv"

  @property
  def dtype(self):
    return self._dtype

  @property
  def algorithm(self):
    return self._algorithm


class Allreduce(Collective):
  def __init__(self, *args, algorithm = None, dtype = None, **kwargs):
    self._algorithm = algorithm or "ring"
    self._dtype = dtype or "float"
    super().__init__(*args, **kwargs)

  @property
  def collective(self):
    return "Allreduce"

  @property
  def dtype(self):
    return self._dtype

  @property
  def algorithm(self):
    return self._algorithm


class Broadcast(Collective):
  def __init__(self, *args, algorithm = None, dtype = None, **kwargs):
    self._algorithm = algorithm or "sendtoall"
    self._dtype = dtype or "float"
    super().__init__(*args, **kwargs)

  @property
  def collective(self):
    return "Broadcast"

  @property
  def dtype(self):
    return self._dtype

  @property
  def algorithm(self):
    return self._algorithm
