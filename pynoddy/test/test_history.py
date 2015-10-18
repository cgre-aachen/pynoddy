import pynoddy.history
from unittest import TestCase

__author__ = 'flow'


class TestHistory(TestCase):

    def test_load_his(self):
        noddy_his = pynoddy.history.NoddyHistory("examples/simple_two_faults.his")
        cube_size = noddy_his.get_cube_size()
        self.assertTrue(cube_size == 100.0)


