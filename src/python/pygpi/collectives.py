import pygpi_wrappers

import inspect
import logging
import numpy as np

def list_implemented_primitives(module, primitive_name):
  classes = inspect.getmembers(module, inspect.isclass)
  return {name : cl for name, cl in classes if name.startswith(f"{primitive_name}_")}

def collective_class_selector(collective, dtype, algorithm):
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
    return "int16"
  elif dtype in [np.int64, "long"]:
    return "long"
  raise ValueError(f"[dtype_to_collective_type] Unknown dtype `{dtype}` for PyGPI primitives")

def Broadcast(*args, algorithm = None, dtype = None, **kwargs):
  collective = "Broadcast"
  algorithm = algorithm or "sendtoall"
  if dtype is None:
    logging.getLogger().warn(f"[pygpi.Broadcast] Setting default dtype to `float`")
    dtype = "float"
  else:
    dtype = dtype_to_collective_type(dtype)
  bcast_class = collective_class_selector(collective, dtype, algorithm)
  return bcast_class(*args, **kwargs)

def Allreduce(*args, algorithm = None, dtype = None, **kwargs):
  collective = "Allreduce"
  algorithm = algorithm or "ring"
  if dtype is None:
    logging.getLogger().warn(f"[pygpi.Allreduce] Setting default dtype to `float`")
    dtype = "float"
  else:
    dtype = dtype_to_collective_type(dtype)
  allreduce_class = collective_class_selector(collective, dtype, algorithm)
  return allreduce_class(*args, **kwargs)
