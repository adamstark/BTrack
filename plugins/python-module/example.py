# need scikits audiolab for reading audio files
from scikits.audiolab import wavread

# import numpy (needed to convert stereo audio to mono)
import numpy as np

# need to import btrack, our beat tracker
import btrack

# set the path to an audio file on your machine
audioFilePath = "/path/to/your/audioFile.wav"

# read the audio file
audioData, fs, enc = wavread (audioFilePath)     # extract audio from file

# convert to mono if need be
if (audioData[0].size == 2):
    print "converting to mono"
    audioData = np.average (audioData, axis = 1)

# ==========================================    
# Usage A: detect beats from audio            
beats = btrack.detect_beats (audioData)    

# ==========================================
# Usage B: extract the onset detection function
odf = btrack.calculate_onset_detection_function (audioData)         

# ==========================================
# Usage C: detect beats from the onset detection function (calculated in Usage B)
odf_beats = btrack.detect_beats_from_odf (odf)
