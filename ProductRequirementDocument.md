# Requirement
## Firmware
* Should be firmware compatible to the [OpenModem](https://unsigned.io/hardware/OpenModem.html), so that the Unsigned.io tools can be used to interact with it.  This also includes the ability to function as a TNC for use with:
  * ~~[APRSdroid](https://aprsdroid.org/)~~ : APRS is an awful protocol to support : inconsistent, inefficient
  * [Sideband for Android](https://unsigned.io/software/Sideband.html).  It's better thought out than APRSdroid and includes encryption.
  * [Codec2_talkie](https://github.com/sh123/codec2_talkie/releases)
* Interfacing with the RNode should be done over Bluetooth.  No wired connections between the RNode and the smartphone.
* Radio interface should be [M17-compatible](https://spec.m17project.org/), or very similar.  9600baud, 4FSK, 12.5kHz channel spacing.

## Hardware
* Bluetooth serial port support
* Battery operated
* Charge via USB-C
* [OHIS-SATA-interface](https://github.com/LieBtrau/ohis-ht-interface?tab=readme-ov-file#sata-interface) to HTs.

# Prior Art
* [M17 Analog Hotspot Gateway Project](https://github.com/nakhonthai/M17AnalogGateway/tree/master):
  * supported hardware: ESP32DR Simple or [ESP32DR_SA818](https://github.com/nakhonthai/ESP32IGate/tree/master/doc/ESP32DR_SA868)
  * using ESP-Arduino development on VScode
  * support M17 mref reflector
  * support noise cancellation
  * support AGC
  * doesn't use an audio codec.  Uses opamp circuits for audio filtering
* [VHF/UHF Transceiver Personality Board](https://g1lro.uk/?p=456)
  * SA868 board with SMD-filter
* [VoltNode UHF repeater](https://github.com/voltlog/VoltNode/blob/main/VoltNode%20UHF%20repeater%20schematic%20revB.pdf)