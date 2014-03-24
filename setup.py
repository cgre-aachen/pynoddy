import os
from setuptools import setup

# Advise from https://pythonhosted.org/an_example_pypi_project/setuptools.html
# Utility function to read the README file.
# Used for the long_description.  It's nice, because now 1) we have a top level
# README file and 2) it's easier to type in the README file than to put a raw
# string in below ...
def read(fname):
    return open(os.path.join(os.path.dirname(__file__), fname)).read()


setup(name='pynoddy',
      version='0.1',
      description='Python wrappers for kinematic geological simulations with Noddy',
      long_description=read('README.md'),
      classifiers=[
          "Development Status :: 2 - Pre-Alpha",
          "Intended Audience :: Science/Research",
          "Programming Language :: Python :: 2.7",
          "Intended Audience :: Education",
          "Topic :: Education",
          "Topic :: Scientific/Engineering",
          "Topic :: Scientific/Engineering :: Physics",
          "Topic :: Scientific/Engineering :: Visualization",
          "License :: OSI Approved :: MIT License",
      ],
      url='http://github.com/flohorovicic/pynoddy',
      author='Florian Wellmann',
      author_email='flohorovicic@gmail.com',
      license='MIT',
      packages=['pynoddy'],
      # package_dir = {'output' : 'pynoddy', 'history' : 'pynoddy'},
      py_modules = ['pynoddy.history', 'pynoddy.output'],
      zip_safe=False)
