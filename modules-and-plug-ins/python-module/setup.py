# setup.py
# build command : python setup.py build build_ext --inplace
from numpy.distutils.core import setup, Extension
import os, numpy, platform

if platform.machine() == 'arm64':
      include_path = '/opt/homebrew/include'
      library_path = '/opt/homebrew/lib'
else:
      include_path = '/usr/local/include'
      library_path = '/usr/local/lib'

name = 'btrack'
sources = [
      'btrack_python_module.cpp',
      '../../src/OnsetDetectionFunction.cpp',
      '../../src/BTrack.cpp'
      ]

sources.append ('../../libs/kiss_fft130/kiss_fft.c')

include_dirs = [
      numpy.get_include(),
      include_path
]

include_dirs.append ('../../libs/kiss_fft130')

setup(name = 'BTrack',
      include_dirs = include_dirs,
      ext_modules = [Extension(name, sources,libraries = ['fftw3','samplerate'],library_dirs = [library_path],define_macros=[
                         ('USE_FFTW', None)])]
      )