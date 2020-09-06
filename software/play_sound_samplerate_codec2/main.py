"""
Play a wav file.  If needed it will be up-sampled.
"""
import pyaudio
import wave
import sys
import numpy as np
import samplerate
import time
import pycodec2

AUDIO_SAMPLE_RATE = 48000  # must be a value supported by the sound card (48000)

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

# Create the Codec2 object
c2 = pycodec2.Codec2(1200)

# Set the buffer size of the audio output buffer.
chunk = int(c2.samples_per_frame() * AUDIO_SAMPLE_RATE / wf.getframerate())


def callback(in_data, frame_count, time_info, status):
    """
    :param in_data:
    :param frame_count: output should contain this amount of samples (=chunk )
    :param time_info:
    :param status:
    :return: the required number of samples in snd_data and the request to continue using pyaudio.paContinue
    """
    # Get 320 speech samples (=640 bytes)
    data = wf.readframes(c2.samples_per_frame())
    if len(data) != c2.samples_per_frame()*2:
        return None, pyaudio.paComplete
    packet = np.frombuffer(data, dtype=np.int16)
    # Codec2 operation
    encoded = c2.encode(packet)
    packet = c2.decode(encoded)

    # Upsample speech from 8kHz to 48kHz : 320 samples become 1920 samples
    snd_data = np.asarray(samplerate.resample(packet, AUDIO_SAMPLE_RATE / wf.getframerate(), 'sinc_best'),
                          dtype=np.int16)

    # Send speech samples to audio output
    return snd_data, pyaudio.paContinue


# List existing sound cards for this hardware
# for ii in range(p.get_device_count()):
#     print(str(ii) + '\t' + p.get_device_info_by_index(ii).get('name'))

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
