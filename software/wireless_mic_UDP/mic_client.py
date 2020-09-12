#!/usr/bin/env python3
"""
Connect to the audio source server.
Output audio to the sound card.
"""
import pyaudio
import socket
import sys
import time

FORMAT = pyaudio.paInt16
CHANNELS = 1
RATE = 48000
CHUNK = 1024
sound_card = 0
LISTEN_PORT = 4444


def callback(in_data, frame_count, time_info, status):
    out_data, addr = s.recvfrom(CHUNK * audio.get_sample_size(FORMAT))
    # out_data is a byte array whose length should be the (frame_count * channels * bytes-per-channel) if output=True
    return out_data, pyaudio.paContinue


# Create a socket
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(('', LISTEN_PORT))

# Connect to the server (hostname, port)

audio = pyaudio.PyAudio()
# Create an audio object for audio sink (=output)
stream = audio.open(
    format=FORMAT,
    channels=CHANNELS,
    rate=RATE,
    output=True,
    output_device_index=sound_card,
    frames_per_buffer=CHUNK,
    stream_callback=callback,
    )

try:
    while stream.is_active():
        time.sleep(0.1)
except KeyboardInterrupt:
    pass

print('Shutting down')
s.close()
stream.close()
audio.terminate()
