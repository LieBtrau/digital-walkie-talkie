# codec2 on platformio

This is a proof-of-concept for using the [Codec2 audio codec library](http://www.rowetel.com/?page_id=452) on the Arduino-compatible [Nodemcu-32s](https://wiki.keyestudio.com/KS0413_keyestudio_ESP32_Core_Board). 

The development started originally using the [codec2-arduino](https://github.com/blanu/codec2-arduino) library, but there happened to be a lot of issues with that code.  Encoding didn't work.  Once that was fixed, it didn't yield the same encoded data as current codec2 implementations.  Time for tabula rasa and start all over again based on version 0.9.2 of the code in [the official codec2 repository](https://github.com/drowe67/codec2).
For some unknown reason, this implementation doesn't yield the same encoded data as the Linux build.  The encoded data of this implementation can be decoded using the Linux build of c2dec.  The result still sounds intelligible.  There's currently no incentive to dive deeper into this.

Due to a lack of flash space on the ESP32, the 700C, 450 and 450PWB modes had to be disabled.  See platformio.ini for the details.

## Getting Started

This is just a proof-of-concept. 

### Prerequisites

Codec2 requires about 87KB of flash.

The default main.cpp in the Arduino framework only gives the main loop about 8KB of RAM.  You must manually edit this file and change that to at least 24KB.  Otherwise you'll run into stack overflow trouble.

### Installing


## Performance
### mode 1200bps (40ms audio packet)
|  | Encoding [ms]| Decoding [ms]|
|--|--|--|
|ESP32  | 16.8 | 15.2|
|Adafruit Metro M4 | 22 | 22 |

## Contributing

This is just a proof of concept. Better examples would be great to have. If you would be interested in working on that, let me know.

## Authors

* **Christoph Tack** - *Initial concept*

## License

The code is licensed under the GPL v3.0 license - see the [LICENSE.md](LICENSE.md) file for details

Codec2 is licensed under the LGPL v2.1 license


## Acknowledgments


