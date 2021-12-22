import pygpi
import pytest

import numpy as np
import random
import string

class TestBroadcast:

  def execute_broadcast(self, number_elements, root, dtype, input_buffer):
    broadcast = pygpi.Broadcast(pygpi.Group(), number_elements, root,
                                dtype = dtype)
    if pygpi.get_rank() == root:
      broadcast.start(input_buffer)
    else:
      broadcast.start()
    output_array = broadcast.wait_for_completion()
    return output_array

  @pytest.mark.parametrize("list_length", [0, 1, 23, 1000])
  @pytest.mark.parametrize("dtype", [float, int])
  def test_list_identical_inputs(self, list_length, dtype):
    root = 0
    input_list = [ dtype(3.456) ] * list_length
    expected_output_array = input_list

    output_array = self.execute_broadcast(len(input_list), root, dtype, input_list)
    assert isinstance(output_array, np.ndarray)
    assert np.array_equal(output_array, expected_output_array)

  @pytest.mark.parametrize("array_length", [0, 8, 35, 67002])
  @pytest.mark.parametrize("dtype", [np.float, np.double, np.float32, np.int32, np.int16])
  def test_array_different_inputs(self, array_length, dtype):
    root = pygpi.get_size()-1
    input_array = np.array([idx + root for idx in range(array_length)], dtype = dtype)
    expected_output_array = input_array

    output_array = self.execute_broadcast(len(input_array), root, dtype, input_array)
    assert isinstance(output_array, np.ndarray)
    assert np.array_equal(output_array, expected_output_array)

  def test_single_value(self):
    root = pygpi.get_size() - 1
    dtype = float
    input_value = dtype(root)
    expected_output = input_value

    output_array = self.execute_broadcast(1, root, dtype, input_value)
    assert isinstance(output_array, np.ndarray)
    assert len(output_array) == 1
    assert expected_output == output_array[0]

  @pytest.mark.parametrize("list_length", [0, 1001])
  @pytest.mark.parametrize("dtype", ["int", "long"])
  @pytest.mark.parametrize("algorithm", ["linear", "sendtoall"])
  def test_algorithms(self, list_length, dtype, algorithm):
    root = 0
    input_list = [ pygpi.get_size() ] * list_length
    expected_output = input_list
    if pygpi.get_rank() != root:
      input_list = None

    broadcast = pygpi.Broadcast(pygpi.Group(), list_length, root = 0,
                                dtype = dtype, algorithm = algorithm)
    broadcast.start(input_list)
    output_array = broadcast.wait_for_completion()
    assert np.array_equal(output_array, expected_output)
