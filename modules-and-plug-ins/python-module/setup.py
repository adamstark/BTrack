# setup.py
# build command : python setup.py build build_ext --inplace
from numpy.distutils.core import setup, Extension
import os, numpy

name = 'btrack'
sources = ['btrack_python_module.cpp','../../src/OnsetDetectionFunction.cpp','../../src/BTrack.cpp']

sources.append ('../../libs/kiss_fft130/kiss_fft.c')

include_dirs = [
                numpy.get_include(),'/usr/local/include'
                ]

include_dirs.append ('../../libs/kiss_fft130')

setup( name = 'BTrack',
      include_dirs = include_dirs,
      ext_modules = [Extension(name, sources,libraries = ['fftw3','samplerate'],library_dirs = ['/usr/local/lib'],define_macros=[
                         ('USE_KISS_FFT', None)])]
      )