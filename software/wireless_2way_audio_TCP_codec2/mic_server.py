#!/usr/bin/env python3
"""
Read audio from the sound card
Send the audio to the connected clients
Based on : https://gist.github.com/fopina/3cefaed1b2d2d79984ad7894aef39a68
"""

import numpy as np
import pyaudio
import socket
import select
import codec2_func

FORMAT = pyaudio.paInt16
CHANNELS = 1
SERVER_PORT = 4444
MAXIMUM_NR_OF_CLIENTS = 1
sound_card = 0
CHUNK = codec2_func.chunk

audio = pyaudio.PyAudio()

# Create an AddressFamily_IPv4 socket and type SOCK_STREAM (TCP), not SOCK_DGRAM (UDP)
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
# Bind to port 4444 on all interfaces
server_socket.bind(('', SERVER_PORT))
# Server will start accepting connections.  Maximum 5 clients allowed
server_socket.listen(MAXIMUM_NR_OF_CLIENTS)


# Set the buffer size of the audio output buffer.


def callback(in_data, frame_count, time_info, status):
    out_data = bytes()
    in_data = codec2_func.encode_codec2(in_data)
    # send data to all the clients.  Remember that the server itself is index 0 of read_list.
    for s in read_list[1:]:
        s.send(in_data)
        try:
            if len(out_data) == 0:
                out_data = s.recv(CHUNK * audio.get_sample_size(FORMAT))
                out_data = codec2_func.decode_codec2(out_data)
        except OSError:
            pass
    if len(out_data) < CHUNK:
        out_data = np.asarray(bytearray(CHUNK), np.int16)
    return out_data, pyaudio.paContinue


# start Recording
stream = audio.open(format=FORMAT,
                    channels=CHANNELS,
                    rate=codec2_func.AUDIO_SAMPLE_RATE,
                    input=True,
                    input_device_index=sound_card,
                    output=True,
                    output_device_index=sound_card,
                    frames_per_buffer=CHUNK,
                    stream_callback=callback)

# List of items that can be read
read_list = [server_socket]
print("recording...")

try:
    while True:
        # check which items of the read_list are ready for reading
        readable, writable, errored = select.select(read_list, [], [])
        for s in readable:
            if s is server_socket:
                # the server_socket is ready to be read from
                (client_socket, address) = server_socket.accept()
                # a new client has connected to the server, add it to the read_list
                read_list.append(client_socket)
                print("Connection from", address)
except KeyboardInterrupt:
    pass

print("finished recording")

server_socket.close()
# stop Recording
stream.stop_stream()
stream.close()
audio.terminate()
