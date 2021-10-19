import pygpi
import pytest

import inspect
import numpy as np

class TestBroadcast:
  def test_bcast(self):

    buffer = np.ones(15, dtype=np.int32)
    root = 0
    group = pygpi.Group()
    bcast = pygpi.Broadcast(group, buffer.size, root, dtype = "int")

    print (f"buffer={buffer}")
    if pygpi.get_rank() == root:
      bcast.start(buffer)
    else:
      bcast.start(input = None)

    result = bcast.waitForCompletion()
    print(f"[rank {pygpi.get_rank()}] result = {result}")
    assert np.array_equal(result, buffer)
