import time

import btrack
import numpy as np
from scipy.io import wavfile

# set the path to an audio file on your machine
audio_file_path = "/path/to/your/audioFile.wav"
_, audio_data = wavfile.read(audio_file_path)

# convert to mono if need be
if audio_data.shape[1] == 2:
    print("converting to mono")
    audio_data = np.average(audio_data, axis=1)

# ==========================================
# Usage A: track beats from audio
t0 = time.time()
beats = btrack.trackBeats(audio_data)
print(f"Calculated beats using btrack.trackBeats function in {round(time.time() - t0, 2)}s")
print(beats)

# ==========================================
# Usage B: extract the onset detection function
t0 = time.time()
onsetDF = btrack.calculateOnsetDF(audio_data)
ODFbeats = btrack.trackBeatsFromOnsetDF(onsetDF)
print(f"Calculated beats using onset detection function in {round(time.time() - t0, 2)}s")
print(ODFbeats)
