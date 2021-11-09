import pygpi
import pytest

import numpy as np

class TestAllreduce:

  @pytest.mark.parametrize("list_length", [0, 1, 23, 1000])
  @pytest.mark.parametrize("dtype", [float, int])
  def test_list_identical_inputs(self, list_length, dtype):
    input_list = [ dtype(3.456) ] * list_length
    expected_output_array = [ elem * pygpi.get_size() for elem in input_list]

    allreduce = pygpi.Allreduce(pygpi.Group(), len(input_list), pygpi.ReductionOp.SUM,
                                dtype = dtype)
    allreduce.start(input_list)
    output_array = allreduce.wait_for_completion()

    assert isinstance(output_array, np.ndarray)
    assert np.array_equal(output_array, expected_output_array)

  @pytest.mark.parametrize("array_length", [0, 8, 35, 67002])
  @pytest.mark.parametrize("dtype", [np.float, np.double, np.float32, np.int32, np.int16])
  def test_array_different_inputs(self, array_length, dtype):
    input_array = np.empty(shape=(array_length), dtype=dtype)
    input_array.fill(pygpi.get_rank())

    expected_output_array = np.empty(input_array.shape, dtype=dtype)
    expected_output_array.fill(sum(range(pygpi.get_size())))

    allreduce = pygpi.Allreduce(pygpi.Group(), input_array.size, pygpi.ReductionOp.SUM,
                                dtype = dtype)
    allreduce.start(input_array)
    output_array = allreduce.wait_for_completion()

    assert isinstance(output_array, np.ndarray)
    assert np.array_equal(output_array, expected_output_array)

  def test_single_value(self):
    dtype = float
    input_value = dtype(pygpi.get_rank())
    expected_output = sum(range(pygpi.get_size()))

    allreduce = pygpi.Allreduce(pygpi.Group(), 1, pygpi.ReductionOp.SUM,
                                dtype = dtype)
    allreduce.start(input_value)
    output = allreduce.wait_for_completion()

    assert isinstance(output, np.ndarray)
    assert len(output) == 1
    assert expected_output == output[0]

  @pytest.mark.parametrize("list_length", [0, 1001])
  @pytest.mark.parametrize("dtype", ["int", "double"])
  @pytest.mark.parametrize("algorithm", ["ring", "recursivedoubling"])
  def test_algorithms(self, list_length, dtype, algorithm):
    input_list = [ pygpi.get_size() ] * list_length
    expected_output = [elem * pygpi.get_size() for elem in input_list]

    allreduce = pygpi.Allreduce(pygpi.Group(), list_length, pygpi.ReductionOp.SUM,
                                dtype = dtype, algorithm = algorithm)
    allreduce.start(input_list)
    output_array = allreduce.wait_for_completion()
    assert np.array_equal(output_array, expected_output)
