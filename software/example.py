# See README.md for documentation.
import struct
import sys

import numpy as np
import pycodec2

if __name__ == '__main__':
    input = sys.argv[1]
    c2 = pycodec2.Codec2(1200)
    INT16_BYTE_SIZE = 2
    PACKET_SIZE = c2.samples_per_frame() * INT16_BYTE_SIZE

    with open(sys.argv[1], 'rb') as input,\
         open(sys.argv[2], 'wb') as output:
        while True:
            packet = input.read(PACKET_SIZE)
            if len(packet) != PACKET_SIZE:
                break
            packet = np.frombuffer(packet, dtype=np.int16)
            encoded = c2.encode(packet)
            # output.write(encoded)
            packet = c2.decode(encoded)
            output.write(np.asarray(packet,dtype=np.int16))
