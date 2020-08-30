#!/usr/bin/env python3
"""
Connect to the audio source server.
Output audio to the sound card.
https://gist.github.com/fopina/3cefaed1b2d2d79984ad7894aef39a68
Run locally using : python3 mic_client.py 127.0.0.1
Run remotely to Wandboard mic_server using : python3 mic_client.py wandboard.local
"""
import pyaudio
import socket
import sys

FORMAT = pyaudio.paInt16
CHANNELS = 1
RATE = 48000
CHUNK = 4096
sound_card = 0
SERVER_PORT = 4444

# Create a socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# Connect to the server (hostname, port)
s.connect((sys.argv[1], SERVER_PORT))
audio = pyaudio.PyAudio()
# Create an audio object for audio sink (=output)
stream = audio.open(
    format=FORMAT,
    channels=CHANNELS,
    rate=RATE,
    output=True,
    output_device_index=sound_card,
    frames_per_buffer=CHUNK)

try:
    while True:
        # Read data from the socket
        data = s.recv(CHUNK)
        # Send it to the audio sink
        stream.write(data)
except KeyboardInterrupt:
    pass

print('Shutting down')
s.close()
stream.close()
audio.terminate()
