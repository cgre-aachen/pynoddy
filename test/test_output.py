from unittest import TestCase
import numpy as np
import pynoddy.output
from nose.tools import assert_equals
import os

__author__ = 'flow'

package_directory = os.path.dirname(os.path.abspath(__file__))
print package_directory


class TestOutput(TestCase):

    def test_load_output(self):
        output_name = os.path.join(package_directory, "../test/simple_two_faults_out")
        noddy_out = pynoddy.output.NoddyOutput(output_name)
        out_shape = np.shape(noddy_out.block)
        assert_equals(out_shape, (124, 94, 50))
        out_id = noddy_out.block[10,10,10]
        assert_equals(out_id, 8.0)
