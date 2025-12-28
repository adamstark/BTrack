import btrack_beat_tracker
from test_data import *

"""
These tests are really just to check that the
Python module is doing something sensible (on the 
many platforms and versions of Python we might be using)
and not producing junk before we publish to PyPI. So this isn't 
expected to be (yet) a full set of tests.
"""

# helper function for adding arrays to test_data.py
def print_array(data):
  print(np.array2string(
          data,
          separator=", ",
          max_line_width=np.inf,
          threshold=np.inf,
          precision=17
      ))

def test_version():
    assert hasattr(btrack_beat_tracker, "__version__")
    assert isinstance(btrack_beat_tracker.__version__, str)
    assert len(btrack_beat_tracker.__version__) > 0

def test_beats_from_odf1():
  beats1 = btrack_beat_tracker.detect_beats_from_odf (odf1)
  assert len(beats1) == 94
  assert np.allclose(beats1, expected_beats1, rtol=1e-6, atol=1e-8)

def test_beats_from_odf2():
  beats2 = btrack_beat_tracker.detect_beats_from_odf (odf2)
  assert len(beats2) == 110
  assert np.allclose(beats2, expected_beats2, rtol=1e-6, atol=1e-8)

def test_beats_from_odf3():
  beats3 = btrack_beat_tracker.detect_beats_from_odf (odf3)
  assert len(beats3) == 126
  assert np.allclose(beats3, expected_beats3, rtol=1e-6, atol=1e-8)


  