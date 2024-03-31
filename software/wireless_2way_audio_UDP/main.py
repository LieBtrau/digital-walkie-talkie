# https://stackoverflow.com/questions/21164804/udp-sound-transfer-played-sound-have-big-noise

#!/usr/bin/env python3
"""
Connect to the audio source server.
Output audio to the sound card.
Based on : https://gist.github.com/fopina/3cefaed1b2d2d79984ad7894aef39a68

Run remotely to Wandboard mic_server using : python3 main.py wandboard.local
Don't run this locally (mic_server and mic_client) running on the same machine.  That works well on the Wandboard,
but not on the i7 laptop.
"""
import pyaudio
import socket
import sys
import time
import select

FORMAT = pyaudio.paInt16
CHANNELS = 1
RATE = 48000
CHUNK = 1024
sound_card = 0
LISTEN_PORT = 4444



def callback(in_data, frame_count, time_info, status):
    out_data = bytes()
    readable, writable, errored = select.select([s], [], [], 0.1)
    for k in readable:
        out_data, addr = k.recvfrom(CHUNK * audio.get_sample_size(FORMAT))

    if len(out_data) < CHUNK * audio.get_sample_size(FORMAT):
        ba = bytearray(out_data)
        ba.extend(bytes(CHUNK * audio.get_sample_size(FORMAT) - len(out_data)))
        out_data = bytes(ba)
    s.sendto(in_data, (sys.argv[1], LISTEN_PORT))
    # out_data is a byte array whose length should be the (frame_count * channels * bytes-per-channel) if output=True
    print(len(out_data))
    return out_data, pyaudio.paContinue


audio = pyaudio.PyAudio()
# Create a socket
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(('', LISTEN_PORT))

# Create an audio object for audio sink (=output)
stream = audio.open(
    format=FORMAT,
    channels=CHANNELS,
    rate=RATE,
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
