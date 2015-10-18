import pynoddy
from unittest import TestCase
from nose.tools import assert_equals
import os

__author__ = 'flow'

package_directory = os.path.dirname(os.path.abspath(__file__))
print package_directory

class TestHistory(TestCase):
    def test_compute_model(self):
        history = os.path.join(package_directory, "../test/simple_two_faults.his")
        output_name = os.path.join(package_directory, "../test/simple_two_faults_out")
        return_val = pynoddy.compute_model(history, output_name)
        assert_equals(return_val, "", msg="Problem with Noddy computation: %s" % return_val)
