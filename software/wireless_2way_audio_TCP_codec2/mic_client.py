#!/usr/bin/env python3
"""
Connect to the audio source server.
Output audio to the sound card.
Based on : https://gist.github.com/fopina/3cefaed1b2d2d79984ad7894aef39a68

Run remotely to Wandboard mic_server using : python3 mic_client.py wandboard.local
Don't run this locally (mic_server and mic_client) running on the same machine.  That works well on the Wandboard,
but not on the i7 laptop.
"""

import numpy as np
import pyaudio
import socket
import sys
import time
import codec2_func

FORMAT = pyaudio.paInt16
CHANNELS = 1
CHUNK = codec2_func.chunk
sound_card = 0
SERVER_PORT = 4444


def callback(in_data, frame_count, time_info, status):
    in_data = codec2_func.encode_codec2(in_data)
    s.sendall(in_data)

    # out_data is a byte array whose length should be the (frame_count * channels * bytes-per-channel) if output=True
    out_data = s.recv(CHUNK * audio.get_sample_size(FORMAT))
    out_data = codec2_func.decode_codec2(out_data)
    if len(out_data) < CHUNK:
        out_data = np.asarray(bytearray(CHUNK), np.int16)
    return out_data, pyaudio.paContinue


# Create a socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# Connect to the server (hostname, port)
s.connect((sys.argv[1], SERVER_PORT))
audio = pyaudio.PyAudio()
# Create an audio object for audio sink (=output)
stream = audio.open(
    format=FORMAT,
    channels=CHANNELS,
    rate=codec2_func.AUDIO_SAMPLE_RATE,
    input=True,
    input_device_index=sound_card,
    output=True,
    output_device_index=sound_card,
    frames_per_buffer=CHUNK,
    stream_callback=callback,
)

try:
    while stream.is_active():
        time.sleep(1)
except KeyboardInterrupt:
    pass

print('Shutting down')
s.close()
stream.close()
audio.terminate()
