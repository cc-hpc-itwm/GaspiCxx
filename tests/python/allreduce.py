import pygpi
import pytest

import numpy as np

class TestAllreduce:
  def test_allreduce(self):

    buffer = np.ones(15, dtype=np.float32)
    root = 0
    allreduce = pygpi.Allreduce(pygpi.Group(), buffer.size, pygpi.ReductionOp.SUM)

    print (f"buffer={buffer}")
    allreduce.start(buffer)

    result = allreduce.wait_for_completion()
    expected = buffer * pygpi.get_size()
    print(f"[rank {pygpi.get_rank()}] result = {result}")
    assert np.array_equal(result, expected)
