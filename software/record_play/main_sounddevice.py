#!/usr/bin/env python3
"""
Pass input analog audio directly to analog output.
https://pypi.org/project/sounddevice/
See https://www.assembla.com/spaces/portaudio/subversion/source/HEAD/portaudio/trunk/test/patest_wire.c
"""

#import resampy

AUDIO_SAMPLE_RATE = 48000
CHUNK_SIZE = 1020  # must be a multiple of (48000 / 8000 = 6)
CODEC2_SAMPLE_RATE = 8000
sound_card = 2

try:
    import sounddevice as sd

    def callback(indata, outdata, frames, time, status):
        if status:
            print(status)
        outdata[:, 1] = indata[:, 1]
 #       y_8000 = resampy.resample(indata[:, 1], AUDIO_SAMPLE_RATE, CODEC2_SAMPLE_RATE)
 #       outdata[:, 1] = resampy.resample(y_8000, CODEC2_SAMPLE_RATE, AUDIO_SAMPLE_RATE)

    with sd.Stream(device=(sound_card, sound_card),
                   samplerate=AUDIO_SAMPLE_RATE, blocksize=CHUNK_SIZE,
                   dtype='int16', latency=0.1,
                   channels=2, callback=callback):
        print('#' * 80)
        print('press Return to quit')
        print('#' * 80)
        input()
except KeyboardInterrupt:
    exit(0)
except Exception as e:
    print(e)
else:
    print("nothing happened")
