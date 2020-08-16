#!/usr/bin/env python3
"""
asynchronously record 5s of audio and output it again
"""
import pyaudio
import time

CHUNK_SIZE = 1024
FORMAT = pyaudio.paInt16
RATE = 44100
sound_card = 0
timeout = 30


def callback(in_data, frame_count, time_info, status):
    return in_data, pyaudio.paContinue


if __name__ == '__main__':
    print("please speak a word into the microphone")
    p = pyaudio.PyAudio()
    stream = p.open(format=FORMAT,
                    channels=1, rate=RATE,
                    input=True,
                    input_device_index=sound_card,
                    output=True,
                    output_device_index=sound_card,
                    frames_per_buffer=CHUNK_SIZE,
                    stream_callback=callback)
    t0 = time.perf_counter()
    while stream.is_active():
        if time.perf_counter() - t0 > timeout:
            break
    stream.stop_stream()
    stream.close()
    p.terminate()
