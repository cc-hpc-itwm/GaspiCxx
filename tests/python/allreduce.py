import pygpi
import pytest

import numpy as np

class TestAllreduce:

  @pytest.mark.parametrize("list_length", [0, 1, 23, 1000])
  @pytest.mark.parametrize("dtype", [float, int])
  @pytest.mark.parametrize("algorithm", ["ring", "recursivedoubling"])
  def test_list_algorithms(self, list_length, dtype, algorithm):
    input_list = [ dtype(3.456), dtype(100), dtype(-4) ] * list_length
    expected_output_array = [ elem * pygpi.get_size() for elem in input_list]

    allreduce = pygpi.Allreduce(pygpi.Group(), len(input_list), pygpi.ReductionOp.SUM,
                                dtype = dtype, algorithm = algorithm)
    allreduce.start(input_list)
    output_array = allreduce.wait_for_completion()

    assert isinstance(output_array, np.ndarray)
    assert np.array_equal(output_array, expected_output_array)

  @pytest.mark.parametrize("array_length", [1, 10, 13])
  @pytest.mark.parametrize("dtype", [bool])
  @pytest.mark.parametrize("reduction_op", [pygpi.ReductionOp.AND, pygpi.ReductionOp.OR])
  @pytest.mark.parametrize("algorithm", ["ring", "recursivedoubling"])
  def test_array_algorithms_bool(self, array_length, dtype, reduction_op, algorithm):
    input_array = np.empty(shape=(array_length), dtype=dtype)
    input_array.fill( pygpi.get_rank() > 0 )

    expected_output_array = np.empty(input_array.shape, dtype=dtype)
    if reduction_op == pygpi.ReductionOp.AND:
      expected_output_array.fill(all(range(pygpi.get_size())))
    elif reduction_op == pygpi.ReductionOp.OR:
      expected_output_array.fill(any(range(pygpi.get_size())))

    allreduce = pygpi.Allreduce(pygpi.Group(), input_array.size, reduction_op,
                                dtype = dtype, algorithm = algorithm)
    allreduce.start(input_array)
    output_array = allreduce.wait_for_completion()

    assert isinstance(output_array, np.ndarray)
    assert output_array.dtype == dtype
    assert np.array_equal(output_array, expected_output_array)

  @pytest.mark.parametrize("array_length", [0, 8, 35, 67002])
  @pytest.mark.parametrize("dtype", [np.float, np.double, np.float32, float,
                                     np.int32, np.int16, np.int8, int])
  def test_array_different_inputs_dtypes(self, array_length, dtype):
    input_array = np.empty(shape=(array_length), dtype=dtype)
    input_array.fill(pygpi.get_rank())

    expected_output_array = np.empty(input_array.shape, dtype=dtype)
    expected_output_array.fill(sum(range(pygpi.get_size())))

    allreduce = pygpi.Allreduce(pygpi.Group(), input_array.size, pygpi.ReductionOp.SUM,
                                dtype = dtype)
    allreduce.start(input_array)
    output_array = allreduce.wait_for_completion()

    assert isinstance(output_array, np.ndarray)
    assert output_array.dtype == dtype
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

  @pytest.mark.parametrize("dtype", ["int", "float"])
  def test_reduction_PROD(self, dtype):
    list_length = 30
    input_list = [ pygpi.get_size() ] * list_length
    expected_output = [elem ** pygpi.get_size() for elem in input_list]

    allreduce = pygpi.Allreduce(pygpi.Group(), list_length, pygpi.ReductionOp.PROD,
                                dtype = dtype)
    allreduce.start(input_list)
    output_array = allreduce.wait_for_completion()
    assert np.array_equal(output_array, expected_output)

  @pytest.mark.parametrize("dtype", [int, float])
  def test_reduction_MIN(self, dtype):
    list_reps = 20
    input_list = [pygpi.get_rank(), -5 * pygpi.get_rank(), 3 * pygpi.get_rank()] * list_reps
    expected_output = [ 0, -5 * (pygpi.get_size() - 1), 0] * list_reps

    allreduce = pygpi.Allreduce(pygpi.Group(), len(input_list), pygpi.ReductionOp.MIN,
                                dtype = dtype)
    allreduce.start(input_list)
    output_array = allreduce.wait_for_completion()
    assert np.array_equal(output_array, expected_output)

  @pytest.mark.parametrize("dtype", ["int", "float"])
  def test_reduction_MAX(self, dtype):
    list_reps = 20
    input_list = [pygpi.get_rank(), -5 * pygpi.get_rank(), 3 * pygpi.get_rank()] * list_reps
    expected_output = [(pygpi.get_size() - 1), 0, 3 * (pygpi.get_size() - 1)] * list_reps

    allreduce = pygpi.Allreduce(pygpi.Group(), len(input_list), pygpi.ReductionOp.MAX,
                                dtype = dtype)
    allreduce.start(input_list)
    output_array = allreduce.wait_for_completion()
    assert np.array_equal(output_array, expected_output)
