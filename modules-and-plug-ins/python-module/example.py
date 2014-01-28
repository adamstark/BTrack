# need scikits audiolab for reading audio files
from scikits.audiolab import wavread

# need to import btrack, our beat tracker
import btrack

# set the path to an audio file on your machine
audioFilePath = "/Users/adamstark/Documents/Audio/Databases/Hainsworth/audio/001.wav"

# read the audio file
audioData, fs, enc = wavread(audioFilePath)     # extract audio from file

# convert to mono if need be
if (audioData[0].size == 2):
    print "converting to mono"
    data = np.average(data,axis=1)

# ==========================================    
# Usage A: track beats from audio            
beats = btrack.trackBeats(audioData)    

# ==========================================
# Usage B: extract the onset detection function
onsetDF = btrack.calculateOnsetDF(audioData)         

# ==========================================
# Usage C: track beats from the onset detection function (calculated in Usage B)
ODFbeats = btrack.trackBeatsFromOnsetDF(onsetDF)
