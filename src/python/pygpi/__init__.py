import pygpi_wrappers

pygpi_wrappers.initPyGPI()

from pygpi_wrappers import get_rank, get_size, ReductionOp
from pygpi_wrappers import Group

from pygpi.collectives import Broadcast
from pygpi.collectives import Allreduce
