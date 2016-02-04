
Test unittest environment for pynoddy
=====================================

.. code:: python

    import unittest
    
    def fun(x):
        return x + 1
    
    class MyTest(unittest.TestCase):
        def test(self):
            self.assertEqual(fun(3), 4)

.. code:: python

    def square(x):
        """Squares x.
    
        >>> square(2)
        4
        >>> square(-2)
        4
        """
    
        return x * x

.. code:: python

    import doctest
    doctest.testmod()




.. parsed-literal::

    TestResults(failed=0, attempted=2)



