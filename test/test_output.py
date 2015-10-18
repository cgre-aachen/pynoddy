from unittest import TestCase
import numpy as np
import pynoddy.output
from nose.tools import assert_equals

__author__ = 'flow'


class TestOutput(TestCase):

    def test_load_output(self):
        noddy_out = pynoddy.output.NoddyOutput("../examples/simple_two_faults_out")
        out_shape = np.shape(noddy_out.block)
        assert_equals(out_shape, (124, 94, 50))
        out_id = noddy_out.block[10,10,10]
        assert_equals(out_id, 8.0)