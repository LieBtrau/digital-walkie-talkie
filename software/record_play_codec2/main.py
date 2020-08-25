#!/usr/bin/env python3
"""
asynchronously record "timeout" seconds of audio and output it again
"""
import numpy as np
import pyaudio
import pycodec2
import samplerate
import time

AUDIO_SAMPLE_RATE = 48000
CODEC2_RATE = 8000
sound_card = 0
timeout = 30


def callback(in_data, frame_count, time_info, status):
    # Parse bytes and convert to array of integers
    packet = np.frombuffer(in_data, dtype=np.int16)
    # Down sample input from 48kHz to 8kHz
    packet = samplerate.resample(packet, CODEC2_RATE / AUDIO_SAMPLE_RATE, 'sinc_medium')
    # Convert array of floats to array of integers
    packet = np.asarray(packet, np.int16)

    # Codec2 operation
    encoded = c2.encode(packet)
    packet = c2.decode(encoded)

    # Up sample speech from 8kHz to 48kHz : 320 samples become 1920 samples
    packet = samplerate.resample(packet, AUDIO_SAMPLE_RATE / CODEC2_RATE, 'sinc_medium')
    # Convert array of floats to array of integers
    snd_data = np.asarray(packet, dtype=np.int16)
    return snd_data, pyaudio.paContinue


if __name__ == '__main__':
    p = pyaudio.PyAudio()
    # Create the Codec2 object
    c2 = pycodec2.Codec2(1200)

    # Set the buffer size of the audio output buffer.
    chunk = int(c2.samples_per_frame() * AUDIO_SAMPLE_RATE / CODEC2_RATE)

    print("please speak a word into the microphone")

    stream = p.open(format=pyaudio.paInt16,
                    channels=1,
                    rate=AUDIO_SAMPLE_RATE,
                    input=True,
                    input_device_index=sound_card,
                    output=True,
                    output_device_index=sound_card,
                    frames_per_buffer=chunk,
                    stream_callback=callback)
    t0 = time.perf_counter()
    while stream.is_active():
        if time.perf_counter() - t0 > timeout:
            break
    stream.stop_stream()
    stream.close()
    p.terminate()
