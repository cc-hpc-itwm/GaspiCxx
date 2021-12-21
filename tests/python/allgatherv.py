import pygpi
import pytest

import numpy as np

class TestAllgatherv:

  @pytest.mark.parametrize("list_length", [0, 1, 23, 1000])
  @pytest.mark.parametrize("dtype", [float, int])
  def test_list_identical_inputs(self, list_length, dtype):
    input_list = [dtype(3.456)] * list_length
    expected_output_array = [elem for elem in input_list] * pygpi.get_size()

    allgatherv = pygpi.Allgatherv(pygpi.Group(), len(input_list),
                                  dtype = dtype)
    allgatherv.start(input_list)
    output_array = allgatherv.wait_for_completion()

    assert isinstance(output_array, np.ndarray)
    assert np.array_equal(output_array, expected_output_array)

  @pytest.mark.parametrize("array_length", [0, 8, 35, 67002])
  @pytest.mark.parametrize("dtype", [np.float, np.double, np.float32, np.int32, np.int16])
  def test_array_different_inputs(self, array_length, dtype):
    input_array = np.empty(shape=(array_length), dtype=dtype)
    input_array.fill(pygpi.get_rank())

    expected_output_array = []
    for rank in range(pygpi.get_size()):
      expected_for_rank = [ rank ] * array_length
      expected_output_array += expected_for_rank

    allgatherv = pygpi.Allgatherv(pygpi.Group(), input_array.size,
                                  dtype = dtype)
    allgatherv.start(input_array)
    output_array = allgatherv.wait_for_completion()

    assert isinstance(output_array, np.ndarray)
    assert np.array_equal(output_array, expected_output_array)

  def test_single_value(self):
    dtype = float
    input_value = dtype(pygpi.get_rank())
    expected_output_array = [value for value in range(pygpi.get_size())]

    allgatherv = pygpi.Allgatherv(pygpi.Group(), 1,
                                  dtype = dtype)
    allgatherv.start(input_value)
    output_array = allgatherv.wait_for_completion()

    assert isinstance(output_array, np.ndarray)
    assert len(output_array) == pygpi.get_size()
    assert np.array_equal(output_array, expected_output_array)

  @pytest.mark.parametrize("list_length", [0, 1001])
  @pytest.mark.parametrize("dtype", ["int", "double"])
  @pytest.mark.parametrize("algorithm", ["ring"])
  def test_algorithms(self, list_length, dtype, algorithm):
    input_list = [ pygpi.get_size() ] * list_length
    expected_output = input_list * pygpi.get_size()
    
    allgatherv = pygpi.Allgatherv(pygpi.Group(), list_length,
                                  dtype = dtype, algorithm = algorithm)
    allgatherv.start(input_list)
    output_array = allgatherv.wait_for_completion()
    assert np.array_equal(output_array, expected_output)