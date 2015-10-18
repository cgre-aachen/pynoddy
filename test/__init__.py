import pynoddy
from unittest import TestCase
from nose.tools import assert_equals

__author__ = 'flow'


class TestHistory(TestCase):
    def test_compute_model(self):
        history = "../examples/simple_two_faults.his"
        output_name = "../examples/simple_two_faults_out"
        output_name = "/Users/flow/git/pynoddy/examples/simple_two_faults_out"
        return_val = pynoddy.compute_model(history, output_name)
        assert_equals(return_val, "", msg="Problem with Noddy computation: %s" % return_val)
