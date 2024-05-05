# Microcontroller
Support for bluetooth serial port profile ([SPP](https://www.bluetooth.com/specifications/specs/serial-port-profile-1-1/)) is needed.  Only the ESP32 has this feature.  There's already support for it in Arduino/PlatformIO as well.

# Wireless Physical Layer
## Technology choice
Using either an software modem with an analog transceiver (probably narrow-band FM) or using a digital module (4(G)FSK)

### Software modem
* Reconfigurable through software: Rattlegram, M17, AFSK, etc.
* High TX-power
* Several bands can be used : HF, VHF, UHF
* On UHF-frequencies, the ham-repeaters can be used to extend the range.
* No RF-design needed, because either connect to HT or use an HT-module (DRA818V, etc.).
* Software modems already available for PC-platform so they can be quickly evaluated without much programming work.
* Requires a lot of processing power.  This should be no problem for the ESP32.

### ~~Digital module~~
* Vendor lock-in.  Si4463 is very-closed source.  CC1200 is more open.
* UHF-frequencies only.
* Incompatible with ham-repeaters (no DCS, CTCSS, etc.).
* The Si4463 and CC1200 are quite complex and require a lot of time to develop a firmware for.
* High-power modules are only available for the Si4463 and only from two Chinese manufacturers.  What if these manufacturers decide to stop manufacturing these modules one day just as they did with the module I'm currently using?
* The transceiver only works in a single band : 434MHz or 868MHz.
* Can use low power MCU
* Lower power consumption

# Analog audio path
An audio codec will be used instead of the ADC/DAC inside the ESP32 because it offers more functions.
* Sampling rate of 8kHz and 48kHz must be supported.
* Microphone input
* Line output

## Options
* ES8311 : Used in ESP32-LyraT-Mini, mono in/out
* ES8388 : Used in Aithinker Audio toolkit, stereo in/out (mono is enough)
