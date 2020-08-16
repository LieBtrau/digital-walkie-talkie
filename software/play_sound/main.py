"""
Play a wav file passed as argument to this program
"""
import pyaudio
import wave
import sys

# length of data to read.
chunk = 1024
# selected sound card
sound_card = 2

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

# List existing sound cards for this hardware
for ii in range(p.get_device_count()):
    print(str(ii) + '\t' + p.get_device_info_by_index(ii).get('name'))

# open stream based on the wave object which has been input.
stream = p.open(format=
                p.get_format_from_width(wf.getsampwidth()),
                channels=wf.getnchannels(),
                rate=wf.getframerate(),
                output=True,
                output_device_index=sound_card)

# read data (based on the chunk size)
data = wf.readframes(chunk)

# play stream (looping from beginning of file to the end)
while data != '':
    # writing to the stream is what *actually* plays the sound.
    stream.write(data)
    data = wf.readframes(chunk)

# cleanup stuff.
stream.close()
p.terminate()
