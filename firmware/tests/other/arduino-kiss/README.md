# Description
Test Arduino KISS with Android apps

# Instruction
1. Download [Codec2_talkie](https://github.com/sh123/codec2_talkie/releases) Android application.
2. Open your download folder and install it from there.
3. Bind the bluetooth of the ESP32 to your Android device
4. Open the Codec2_talkie app and connect to the ESP32.
5. On the serial port of the ESP32 you'll see the loggings from the KISS-interface.

## Receiving data
Codec2 packets will be printed as HEX-data.  You can convert this data to a WAV-file using : 
1. Paste this hexadecimal output in a file named: audio.hex
2. Convert that file to raw bits using : xxd -r -p audio.hex audio.bit
3. Convert that raw bit file to raw audio using (e.g. for codec2-1200): c2dec 1200 audio.bit audio.raw
4. Convert the raw audio file to wav using : sox -e signed-integer -b 16 -r 8000 -c 1 audio.raw audio.wav

## Transmitting data
There's a PTT-button, which is connected to pin 27 by default.  When connecting this pin to GND, then codec2 samples will be sent to the KISS host, who then plays them through the speaker.  The codec2 samples are stored in "lookdave.h".  You have the option of sending multiple codec2 packets in a single super frame.

# Remarks
[M17 KISS HT](https://github.com/mobilinkd/m17-kiss-ht) can't be used.  It only supports Bluetooth-Low-Energy, not classic Bluetooth.
