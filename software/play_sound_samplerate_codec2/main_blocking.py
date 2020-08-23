"""
Play a wav file passed as argument to this program
Using non-blocking code, speed of the sound is too fast.  It's as if only the first half of the audio packets is output.
"""
import pyaudio
import wave
import sys
import numpy as np
import samplerate
import pycodec2

# selected sound card
sound_card = 0
AUDIO_SAMPLE_RATE = 48000  # must be a value supported by the sound card (48000)

# validation. If a wave file hasn't been specified, exit.
if len(sys.argv) < 2:
    print("Plays a wave file.\n\n" + \
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
c2 = pycodec2.Codec2(1200)
INT16_BYTE_SIZE = 2
PACKET_SIZE = c2.samples_per_frame() * INT16_BYTE_SIZE
STRUCT_FORMAT = '{}h'.format(c2.samples_per_frame())

# List existing sound cards for this hardware
for ii in range(p.get_device_count()):
    print(str(ii) + '\t' + p.get_device_info_by_index(ii).get('name'))

# open stream based on the wave object which has been input.
stream = p.open(format=p.get_format_from_width(wf.getsampwidth()),
                channels=wf.getnchannels(),
                rate=AUDIO_SAMPLE_RATE,
                output=True,
                output_device_index=sound_card)

# read data (based on the chunk size)
data = wf.readframes(c2.samples_per_frame())

# play stream (looping from beginning of file to the end)
while data != '':
    packet = np.frombuffer(data, dtype=np.int16)
    encoded = c2.encode(packet)
    packet = c2.decode(encoded)
    snd_data = np.asarray(samplerate.resample(packet, AUDIO_SAMPLE_RATE / wf.getframerate(), 'sinc_best'),
                          dtype=np.int16)
    # writing to the stream is what *actually* plays the sound.
    stream.write(snd_data)
    data = wf.readframes(c2.samples_per_frame())

# cleanup stuff.
stream.close()
p.terminate()
