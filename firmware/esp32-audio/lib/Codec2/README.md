# codec2-arduino

This is a proof-of-concept for using the [Codec2 audio codec library](http://www.rowetel.com/?page_id=452) on the Arduino-compatible [Adafruit Feather nRF52 Bluefruit LE board](https://www.adafruit.com/product/3406). The nRF52 contains a Cortex M4F processor, which is the same one used in the [SM1000](http://www.rowetel.com/?page_id=3902). The SM1000 is a device that encodes and decodes Codec2 audio. It is meant to be used in conjunction with a radio to send and receive voice communications.

Now that Adafruit is providing a convenient Arduino-compatible board with the same processor, I wanted to see if I could get Codec2 to compile inside of the Arduino IDE. It worked!

## Getting Started

This is just a proof-of-concept. It contains the following components:

1. Codec2 source - all of the .h and .c files are included here. The build system is not used.
3. libsamplerate - Required by Codec2. All of the .h and .c files are included here
4. codec2-arduino.ino - An example application

### Prerequisites

You will need an [Adafruit Feather nRF52 Bluefruit LE board](https://www.adafruit.com/product/3406).
The current demo also requires a [Adalogger FeatherWing - RTC + SD Add-on](https://www.adafruit.com/product/2922) and an SD card.

You will also need a USB micro-B cable to program the nRF52 board and some way to write to the SD card from your computer.

### Installing

Install the Codec2 library using the Library Manager in the Arduino IDE.

Open the example application using the Sketchbook -> Examples menu.

Format the SD card to use FAT32.

For testing, you will need a Codec2 file using mode 700B. You can find one in the extras/ directory. Place this on the SD card in the root directory. It must be named TEST700B.C2.

Place the SD card in the Adalogger.

Follow the [Adafruit tutorial](https://learn.adafruit.com/bluefruit-nrf52-feather-learning-guide) to get your nRF52 board set up.

Open the example application using the Sketchbook -> Examples menu.

Connect the USB micro-B cable to the nRF52 board and hit the Upload button.

## Testing

Click the Serial Monitor button in the Arduino IDE.
If all is going well, you should see lots of unreadable gibberish on your screen.
Congratulations, this is successfully decoded audio!

## Contributing

This is just a proof of concept. Better examples would be great to have. If you would be interested in working on that, let me know.

## Authors

* **Dr. Brandon Wiley** - *Initial concept* - [Operator Foundation](https://OperatorFoundation.org/)

## License

The codec2-arduino sketch is licensed under the GPL v3.0 license - see the [LICENSE.md](LICENSE.md) file for details

Codec2 is licensed under the LGPL v2.1 license

libsamplerate is licensed under the 2-clause BSD license

## Acknowledgments

* Thanks to all the folks on the #codec2 IRC channel for helping me figure out how to get everything to compile.
