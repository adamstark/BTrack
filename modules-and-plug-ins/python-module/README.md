# BTrack Python Module

Python bindings for the BTrack beat tracker.

This README explains how to build and install the `btrack` Python module locally.

---

## Prerequisites

- Python 3.8+
- CMake 3.15+
- A C/C++ compiler (Apple Clang on macOS, GCC/Clang on Linux)
- NumPy installed (`pip install numpy`)
- Optional: `libsamplerate` and Kiss FFT if using those backends

---

## 0. Clean any previous builds

  python3 -m pip uninstall btrack -y
  rm -rf build/ dist/ \*.egg-info

## 1. Developer Quick Install

If you want to build and install the module directly for local development:

  cd /path/to/BTrack/modules-and-plug-ins/python-module
  python3 -m pip install --force-reinstall .
