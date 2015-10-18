import pynoddy.history
from unittest import TestCase
import os

__author__ = 'flow'

package_directory = os.path.dirname(os.path.abspath(__file__))
print package_directory

class TestHistory(TestCase):

    def test_load_his(self):
        history = os.path.join(package_directory, "../examples/simple_two_faults.his")
        noddy_his = pynoddy.history.NoddyHistory(history)
        cube_size = noddy_his.get_cube_size()
        self.assertTrue(cube_size == 100.0)


