# BTrack Python Module

Python bindings for the BTrack beat tracker.

This README explains how to build and install the `btrack` Python module locally.

## 1. Installation via PyPI:

```bash
pip install btrack
```

## 2. Build locally

---

## Prerequisites

- Python 3.8+
- CMake 3.15+
- A C/C++ compiler (Apple Clang on macOS, GCC/Clang on Linux)
- NumPy installed (`pip install numpy`)
- Optional: `libsamplerate` and Kiss FFT if using those backends

---

## a) Clean any previous builds

python3 -m pip uninstall btrack -y
rm -rf build/ dist/ \*.egg-info

## b) Developer Quick Install

If you want to build and install the module directly for local development:

cd /path/to/BTrack/modules-and-plug-ins/python-module
python3 -m pip install --force-reinstall .

### 3. **Usage**

TO DO

## 4. License

TO DO
