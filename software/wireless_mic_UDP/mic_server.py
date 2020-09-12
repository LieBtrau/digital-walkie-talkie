#!/usr/bin/env python3
"""
Read audio from the sound card
Send the audio to the client
"""

import pyaudio
import socket
import sys
import time

FORMAT = pyaudio.paInt16
CHANNELS = 1
RATE = 48000
CHUNK = 1024
LISTEN_PORT = 4444
sound_card = 0

audio = pyaudio.PyAudio()
server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)


def callback(in_data, frame_count, time_info, status):
    # send data to all the clients.  Remember that the server itself is index 0 of read_list.
    server_socket.sendto(in_data, (sys.argv[1], LISTEN_PORT))
    return None, pyaudio.paContinue


# start Recording
stream = audio.open(format=FORMAT,
                    channels=CHANNELS,
                    rate=RATE,
                    input=True,
                    input_device_index=sound_card,
                    frames_per_buffer=CHUNK,
                    stream_callback=callback)

print("recording...")

try:
    while True:
        time.sleep(0.1)
except KeyboardInterrupt:
    pass

print("finished recording")

server_socket.close()
# stop Recording
stream.stop_stream()
stream.close()
audio.terminate()
