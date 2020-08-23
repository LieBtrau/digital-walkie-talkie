"""
Play a wav file.  If needed it will be up-sampled.
"""
import audioop

import pyaudio
import wave
import sys
import numpy as np
from array import array
import time

AUDIO_SAMPLE_RATE = 48000  # must be a value supported by the sound card (48000)

# length of data to read.
chunk = 1020  # divisible by 6, which is 48000 / 8000 (if original wave file sampled at 8kHz)
# selected sound card
sound_card = 0

# validation. If a wave file hasn't been specified, exit.
if len(sys.argv) < 2:
    print("Plays a wave file.\n\n" +
          "Usage: %s filename.wav" % sys.argv[0])
    sys.exit(-1)

'''
************************************************************************
      This is the start of the "minimum needed to read a wave"
************************************************************************
'''
# open the file for reading.
wf = wave.open(sys.argv[1], 'rb')

# create an audio object
p = pyaudio.PyAudio()


def callback(in_data, frame_count, time_info, status):
    """
    This callback requests 1020 output samples, which will be output at 48kHz.
    If the input wave file has a 8kHz sample rate, we need to up-sample by a factor of 6.  In that case only
    1020/6 = 170 samples are read from the original wave file.  These are then x6 up-sampled to arrive at the required
    number of 1020 output samples.
    :param in_data:
    :param frame_count: output should contain this amount of samples (=chunk = 1020)
    :param time_info:
    :param status:
    :return: the required number of samples in snd_data and the request to continue using pyaudio.paContinue
    """
    data1 = wf.readframes(int(chunk * wf.getframerate() / AUDIO_SAMPLE_RATE)+1)
    data2, state = audioop.ratecv(data1, 2, 1, wf.getframerate(), AUDIO_SAMPLE_RATE, None)
    return data2, pyaudio.paContinue


# List existing sound cards for this hardware
for ii in range(p.get_device_count()):
    print(str(ii) + '\t' + p.get_device_info_by_index(ii).get('name'))

# open stream based on the wave object which has been input.
stream = p.open(format=p.get_format_from_width(wf.getsampwidth()),
                channels=wf.getnchannels(),
                rate=AUDIO_SAMPLE_RATE,
                output=True,
                frames_per_buffer=chunk,
                output_device_index=sound_card,
                stream_callback=callback)
print("Wav file sample rate: " + str(wf.getframerate()))

while stream.is_active():
    time.sleep(0.1)

# cleanup stuff.
stream.close()
p.terminate()
