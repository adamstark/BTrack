# setup.py
# build command : python setup.py build build_ext --inplace
from numpy.distutils.core import setup, Extension
import os, numpy

name = 'btrack'
sources = ['btrack_python_module.cpp','../src/OnsetDetectionFunction.cpp','../src/BTrack.cpp']

include_dirs = [
                numpy.get_include(),'/usr/local/include'
                ]

setup( name = 'BTtrack',
      include_dirs = include_dirs,
      ext_modules = [Extension(name, sources,libraries = ['fftw3','samplerate'])]
      )