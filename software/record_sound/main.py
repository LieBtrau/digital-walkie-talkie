#!/usr/bin/env python3
"""
asynchronously record 5s of audio and it store to demo.wav
"""

from sys import byteorder
from array import array
from struct import pack
import pyaudio
import wave
import time

samples_in = array('h')

THRESHOLD = 500
CHUNK_SIZE = 1024
FORMAT = pyaudio.paInt16
RATE = 44100
sound_card = 2


def record():
    global samples_in
    """
    Record a word or words from the microphone and
    return the data as an array of signed shorts.

    Normalizes the audio, trims silence from the
    start and end, and pads with 0.5 seconds of
    blank sound to make sure VLC et al can play
    it without getting chopped off.
"""
    p = pyaudio.PyAudio()
    stream = p.open(format=FORMAT,
                    channels=1, rate=RATE,
                    input=True,
                    input_device_index=sound_card,
                    frames_per_buffer=CHUNK_SIZE,
                    stream_callback=callback)

    t0 = time.perf_counter()
    while stream.is_active():
        if time.perf_counter() - t0 > 5:
            break

    sample_width = p.get_sample_size(FORMAT)
    stream.stop_stream()
    stream.close()
    p.terminate()

    samples_in = normalize(samples_in)
    samples_in = add_silence(samples_in, 0.5)
    return sample_width, samples_in


def is_silent(snd_data):
    "Returns 'True' if below the 'silent' threshold"
    return max(snd_data) < THRESHOLD


def normalize(snd_data):
    "Average the volume out"
    MAXIMUM = 16384
    times = float(MAXIMUM) / max(abs(i) for i in snd_data)

    r = array('h')
    for i in snd_data:
        r.append(int(i * times))
    return r


def add_silence(snd_data, seconds):
    "Add silence to the start and end of 'snd_data' of length 'seconds' (float)"
    silence = [0] * int(seconds * RATE)
    r = array('h', silence)
    r.extend(snd_data)
    r.extend(silence)
    return r


def callback(in_data, frame_count, time_info, status):
    # little endian, signed short
    snd_data = array('h', in_data)
    if byteorder == 'big':
        snd_data.byteswap()
    samples_in.extend(snd_data)
    return None, pyaudio.paContinue


def record_to_file(path):
    "Records from the microphone and outputs the resulting data to 'path'"
    sample_width, data = record()
    data = pack('<' + ('h' * len(data)), *data)

    wf = wave.open(path, 'wb')
    wf.setnchannels(1)
    wf.setsampwidth(sample_width)
    wf.setframerate(RATE)
    wf.writeframes(data)
    wf.close()


if __name__ == '__main__':
    print("please speak a word into the microphone")
    record_to_file('demo.wav')
    print("done - result written to demo.wav")
