# need scikits audiolab for reading audio files
from scikits.audiolab import wavread
import numpy as np

# need to import btrack, our beat tracker
import btrack

# set the path to an audio file on your machine
audioFilePath = "/path/to/your/audioFile.wav"

# read the audio file
audioData, fs, enc = wavread(audioFilePath)     # extract audio from file

# convert to mono if need be
if (audioData[0].size == 2):
    print "converting to mono"
    audioData = np.average(audioData,axis=1)

# ==========================================    
# Usage A: track beats from audio            
beats = btrack.trackBeats(audioData)    

# ==========================================
# Usage B: extract the onset detection function
onsetDF = btrack.calculateOnsetDF(audioData)         

# ==========================================
# Usage C: track beats from the onset detection function (calculated in Usage B)
ODFbeats = btrack.trackBeatsFromOnsetDF(onsetDF)
