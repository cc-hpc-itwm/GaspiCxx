import pygpi
import pytest

class TestGroup:
  def test_empty_group(self):
    with pytest.raises(RuntimeError):
      pygpi.Group([])

  def test_empty_group_all_ranks(self):
    g = pygpi.Group()
    assert g.size == pygpi.get_size()

  @pytest.mark.parametrize("ranks_list", [range(pygpi.get_size()),
                                          range(0, pygpi.get_size(), 2)])
  def test_group_contains(self, ranks_list):
    if pygpi.get_rank() in ranks_list:
      g = pygpi.Group(ranks_list)
      assert g.contains_global_rank(pygpi.get_rank())
    else:
      with pytest.raises(RuntimeError):
        pygpi.Group(ranks_list)

  @pytest.mark.parametrize("ranks_list", [range(pygpi.get_size()),
                                          range(0, pygpi.get_size(), 2)])
  def test_group_size(self, ranks_list):
    if pygpi.get_rank() in ranks_list:
      g = pygpi.Group(ranks_list)
      assert g.size == len(ranks_list)
    else:
      with pytest.raises(RuntimeError):
        pygpi.Group(ranks_list)

  @pytest.mark.parametrize("ranks_list", [range(pygpi.get_size()),
                                          range(0, pygpi.get_size(), 2)])
  def test_group_list_of_ranks(self, ranks_list):
    if pygpi.get_rank() in ranks_list:
      g = pygpi.Group(ranks_list)
      assert g.group == list(ranks_list)
    else:
      with pytest.raises(RuntimeError):
        pygpi.Group(ranks_list)
