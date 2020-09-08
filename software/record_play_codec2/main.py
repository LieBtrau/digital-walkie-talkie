#!/usr/bin/env python3
"""
asynchronously record "timeout" seconds of audio and output it again
"""
import pyaudio
import pycodec2
import time
import codec2_func

sound_card = 0
timeout = 30


def callback(in_data, frame_count, time_info, status):
    encoded = codec2_func.encode_codec2(in_data)
    snd_data = codec2_func.decode_codec2(encoded)
    return snd_data, pyaudio.paContinue


if __name__ == '__main__':
    p = pyaudio.PyAudio()
    # Create the Codec2 object
    c2 = pycodec2.Codec2(1200)

    print("please speak a word into the microphone")

    stream = p.open(format=pyaudio.paInt16,
                    channels=1,
                    rate=codec2_func.AUDIO_SAMPLE_RATE,
                    input=True,
                    input_device_index=sound_card,
                    output=True,
                    output_device_index=sound_card,
                    frames_per_buffer=codec2_func.chunk,
                    stream_callback=callback)
    t0 = time.perf_counter()
    while stream.is_active():
        if time.perf_counter() - t0 > timeout:
            break
    stream.stop_stream()
    stream.close()
    p.terminate()
