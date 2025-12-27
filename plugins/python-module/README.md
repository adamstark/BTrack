# BTrack Python Module

Here you can find Python bindings for the BTrack beat tracker.

Full details of the working of the BTrack algorithm can be found in:

* Musicians and Machines: Bridging the Semantic Gap in Live Performance, Chapter 3, A. M. Stark, PhD Thesis, Queen Mary, University of London, 2011.

* Real-Time Beat-Synchronous Analysis of Musical Audio, A. M. Stark, M. E. P. Davies and M. D. Plumbley. In Proceedings of the 12th International Conference on Digital Audio Effects (DAFx-09), Como, Italy, September 1-4, 2009.

## 1. Installation via PyPI:

    pip install btrack_beat_tracker

## 2. **Usage**

### Import btrack

    import btrack_beat_tracker as btrack

### Use Case A: Track beats from audio

`audioData` must be a 1 dimensional numpy array of audio samples (i.e. in mono) at 44100Hz

    beats = btrack.detect_beats (audioData)

`beats` will be the estimated beat times in seconds

### Use Case B: Extract onset detection function

`audioData` must be a 1 dimensional numpy array of audio samples (i.e. in mono) at 44100Hz

    odf = btrack.calculate_onset_detection_function (audioData)

### Use Case C: detect beats from the onset detection function 

The onset detection function is calculated as per Use Case B above

    odf_beats = btrack.detect_beats_from_odf (odf)

## 3. Build locally

### Prerequisites

- Python 3.8+
- CMake 3.15+
- A C/C++ compiler
- NumPy installed (`pip install numpy`)
- `libsamplerate`

### 1) Clean any previous builds

    python3 -m pip uninstall btrack_beat_tracker -y
    rm -rf build/ dist/ \*.egg-info

### 2) Developer Quick Install

If you want to build and install the module directly for local development:

    cd plugins/python-module
    python3 -m pip install --force-reinstall .

## 4. License

Copyright (c) 2014 Queen Mary University of London

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see http://www.gnu.org/licenses/.

See the the main BTrack repository README for full licence details

[https://github.com/adamstark/btrack](https://github.com/adamstark/btrack)
