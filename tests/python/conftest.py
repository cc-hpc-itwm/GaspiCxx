import pytest
import logging
import os

@pytest.fixture(scope="session", autouse = True)
def setup_tests():
  import pygpi
  pygpi.initPyGPI()
