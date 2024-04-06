### Overview of functioning modes
The _Bandwidth_ is dictated by the _Mode_.  The _Offset_ shifts the _Spectrum usage_ in the frequency domain.

|Modulation | Mode | Offset [Hz] | Spectrum usage | Bandwidth [Hz]| Sample duration [s]| Bytes | Bitrate [bps]|
|-----------|------|-------------|----------------|---------------|--------------------|-------|--------------|
| 8PSK | 6    | 1800 |450Hz - 3.15Hz  | 2700    |10 | 5380 | 4304|
| 8PSK | 7    | 1800 |550Hz - 3.05kHz | 2500    |11 | 5380 | 3912|
| QPSK | 8    | 1800 |550Hz - 3.05kHz | 2500    |16 | 5380 | 2690|
| QPSK | 9    | 1800 |700Hz - 2.95kHz | 2250    |18 | 5380 | 2391|
| 8PSK | 10   | 1800 |200Hz - 3.40kHz | 2400    | 9 | 5380 | 4782|
| 8PSK | 11   | 1800 |600Hz - 3.00kHz | 2400    |11 | 5380 | 3912|
| QPSK | 12   | 1450 |250Hz - 2.65kHz | 2400    |18 | 5380 | 2391|
| QPSK | 12   | 1800 |600Hz - 3.00kHz | 2400    |18 | 5380 | 2391|
| QPSK | 13   | 1450 |650Hz - 2.25kHz | 1600    |25 | 5380 | 1721|
|      | 24   | 1500 |                | 1600    | 3.8 | 512 |1077|
|      | 27   | 1500 |                | 1700    | 2 | 512 |2048|
|      | 29   | 1500 |                | 1900    | 1.5 | 512 |2730|

Maximum payload size is 5380 bytes.

More info on [COFDMTV](https://www.aicodix.de/cofdmtv/).

## Other versions
### Short version
The short version creates fix length audio burst of 1.2s.  It either encodes 85 bytes (when your input file is shorter) or 170 bytes of data.  Files larger than 170 bytes will be truncated to 170 bytes.

The modulation is unknown, but it's 1.6kHz wide.  So it's probably a variant of mode 13 of the master-modem.

### Next version
This version allows to encode packets of 256, 512 and 1024 bytes.  The 512 bytes version should be compatible to the Reticulum 500 bytes MTU.

### Installation instructions
```bash
$ mkdir rattlegram-modem
$ cd rattlegram-modem/
$ git clone git@github.com:aicodix/dsp.git
$ git clone git@github.com:aicodix/code.git
$ git clone git@github.com:aicodix/modem.git
$ cd modem/
$ git checkout short
```
Edit `Makefile` and select the g++ compiler instead of clang++
```
#CXX = clang++ -stdlib=libc++ -march=native
CXX = g++ -march=native
```
Run make
```bash
make
```
### Offline test
```bash
$ ./decode decoded.dat encoded.wav 
symbol pos: 2329
coarse cfo: 2000 Hz 
oper mode: 16
call sign: ANONYMOUS
demod .... done
coarse sfo: 0.294881 ppm
finer cfo: 2000 Hz 
Es/N0 (dB): 33.5243 31.3554 30.1929 29.7553
bit flips: 0
```

### Loopback test on USB sound card
```bash
$ pactl set-sink-volume alsa_output.usb-GeneralPlus_USB_Audio_Device-00.analog-stereo 100%
$ pactl set-source-volume alsa_input.usb-GeneralPlus_USB_Audio_Device-00.mono-fallback 25%
$ arecord -c 1 -f S16_LE -r 8000 -d 5 recorded.wav
Recording WAVE 'recorded.wav' : Signed 16 bit Little Endian, Rate 8000 Hz, Mono
```
Play the encoded wave form:
```bash
$ aplay encoded.wav
```
```bash
$ ./decode decoded.dat recorded.wav
symbol pos: 2325
coarse cfo: 2000 Hz 
oper mode: 14
call sign: ANONYMOUS
demod .... done
coarse sfo: 0.217773 ppm
finer cfo: 2000 Hz 
Es/N0 (dB): 30.631 30.4578 30.2292 28.8358
bit flips: 0
```
The USB loopback caused a 1dB to 3dB reduction in SNR.

### Test using radioes and USB sound card 

### Test setup

#### Hardware
This test uses the test setup as found at the end of the [audio level test](./AudioLevelTest.ipynb).

#### Command line
1. Setting the sound card's output audio volume: 
```bash
$ pactl set-sink-volume alsa_output.usb-GeneralPlus_USB_Audio_Device-00.analog-stereo 100%
```
2. Setting the sound card's input audio volume for optimal SNR: 
```bash
$ pactl set-source-volume alsa_input.usb-GeneralPlus_USB_Audio_Device-00.mono-fallback 25%
```
3. Start recording 10s audio sample:
```bash
$ arecord -c 1 -f S16_LE -r 8000 -d 10 recorded.wav
```
4. Push PTT
5. Start playing encoded audio sample:
```bash
$ aplay encoded.wav
```
6. Release PTT after audio sample has played.  Wait for the recording to finish.
7. Decode the recorded audio sample:
```bash
$ ./decode decoded.dat recorded.wav 
symbol pos: 2329
coarse cfo: 2000 Hz 
oper mode: 16
call sign: ANONYMOUS
demod .... done
coarse sfo: -36.853 ppm
finer cfo: 2000.07 Hz 
Es/N0 (dB): 26.1561 25.6838 23.3144 22.5766
bit flips: 0
```
When compared to decoding the original encoded audio, we notice that we lost around 7dB SNR.

----

## M17
* 4GFSK, 9600 baud
* Designed for audio streaming
* doesn't work on PMR446 radios because of the limited audio bandwidth
* works on some HTs with a flat audio response

### Installing M17-tools
```bash
sudo apt install pkg-config libboost-all-dev libgtest-dev libasound-dev codec2
git clone git@github.com:M17-Project/m17-tools.git
cd m17-tools
mkdir build
cd build
cmake .. -DBUILD_GUI_APPS=OFF
make
make test
```
### Testing M17
#### Digital Loopback test
```bash
cd apps
sox ../../ve9qrp.wav -t raw - |  ./m17-mod -S AB1CD -D AB2CD | ./m17-demod -l -d | play -q -b 16 -r 8000 -c1 -t s16 -
```

Reduce audio fragment length:
```bash
sox ../../ve9qrp.wav ../../ve9qrp_10s.wav trim 0 10
```

This can be split into several steps:

1. Create M17 audio file using:
```bash 
sox ../../ve9qrp_10s.wav -t raw - |  ./m17-mod -S AB1CD -D AB2CD -r > ../../ve9qrp_10s_M17.raw
```
This produces a 48kHz 16-bit mono audio file.  

2. Play this file using:
```bash
play -q -b 16 -r 48000 -c1 -t s16 ../../ve9qrp_10s_M17.raw
```

3. For user-friendly playback and analysis with Audacity, convert the raw file to WAV using:
```bash
sox -t raw -r 48000 -b 16 -c 1 -e signed-integer ../../ve9qrp_10s_M17.raw ../../ve9qrp_10s_M17.wav
```
In Audacity, the spectrogram view shows that all the energy is in the frequency range 0-3.6kHz.

4. This M17 data can be decoded and played back using:
```bash
./m17-demod -l -d < ../../ve9qrp_10s_M17.raw | play -q -b 16 -r 8000 -c1 -t s16 -
```

5. The M17 data can be decoded and saved as raw analog audio using:
```bash
./m17-demod -l -d < ../../ve9qrp_10s_M17.raw > ../../ve9qrp_10s_M17_demod.raw
```

#### Loopback through analog audio path
##### Hardware setup
Use a USB sound card.  Use a TRS-cable and connect the speaker output directly to the MIC-input.

##### Setting up REW : Input level
The audio levels of the USB-soundcard must be set correctly.  This can be done using REW.  The following steps are required:

1. Plug in the USB sound card and use a TRS-TRS cable to make an audio loop back.
2. Open REW
2. Select preferences
3. Make sure the tab "Soundcard" is selected
4. Select the USB sound card
  * Output device : default [default]
  * Input device : default [default]
5. Click the button "Calibrate soundcard..."
6. Click "Next>" two times.
7. Set the main volume control of the PC to 100%.  You can do this in three ways:
  * in Ubuntu using the main volume control in the top right corner
  * using command line: `amixer -D pulse sset Master 100%`
  * use command line: `pactl set-sink-volume alsa_output.usb-GeneralPlus_USB_Audio_Device-00.analog-stereo 100%`
8. Adjust the microphone volume control of the USB sound card to get -12dBFS input level.  You can do this in three ways : 
  * Either use the volume control in the sound settings of Ubuntu
  * use command line: `amixer -D pulse sset Capture 43%`
  * Use command line: `pactl set-source-volume alsa_input.usb-GeneralPlus_USB_Audio_Device-00.mono-fallback 43%`

##### Setting up REW : Output level
1. Open REW
2. Select "Levels"
3. Play the M17 data using:
```bash
play -q -b 16 -r 48000 -c1 -t s16 ../../ve9qrp_10s_M17.raw
```
4. Adjust the output level of the USB sound card to get -3dBFS output level.  You can do this in three ways:
  * Either use the volume control in the sound settings of Ubuntu
  * use command line: `amixer -D pulse sset Master 65%`
  * Use command line: `pactl set-sink-volume alsa_output.usb-GeneralPlus_USB_Audio_Device-00.analog-stereo 65%`

##### Software setup
Open a command line for recording the M17-data with a USB sound card.
```bash
rec -c 1 -b 16 -t s16 -r 48000 ../../test.s16 silence 1 0.1 2% 1 3.0 2% 
```
This will start the recording once audio is detected and stop after 3 seconds of silence.

Play the M17 data using:
```bash
play -q -b 16 -r 48000 -c1 -t s16 ../../ve9qrp_10s_M17.raw
```
Finally the M17 data can be decoded and played back using:
```bash
./m17-demod -l < ../../test.s16 | aplay -c 1 -f S16_LE -r 8000 --device plughw:CARD=PCH,DEV=0
```

##### Real time loopback
Open a command line for decoding M17 data and play back the decoded audio:  
```bash
rec -c 1 -b 16 -t s16 -r 48000 - | ./m17-demod -l -d | aplay -c 1 -f S16_LE -r 8000 --device plughw:CARD=PCH,DEV=0
```
Open another command line for encoding audio and transmitting M17 data:  
```bash
sox ../../apollo11_1.wav -t wav - |  ./m17-mod -S AB1CD -D AB2CD -r | play -q -b 16 -r 48000 -c1 -t s16 -
```
There are a lot of decoding errors with this setup.  I'll try looping back through the sound card of my laptop.

#### Loopback through PMR446 radio

##### Hardware setup
See [Audio level test](./doc/PMR446-radio/AudioLevelTest.ipynb)

| Analog audio path | M17 path |
| --- | --- |
| USB sound card | PMR446 radio |
| <img src="./measurements/Spectrogram_analog_audio_loopback_m17.png"/> | <img src="./measurements/Spectrogram_analog_PMR446_loopback_m17.png"/>

##### Results
No audio is being decoded.  Debug info shows that data can not be recovered.

##### Conclusion
It's clear from the spectrograms that these PMR446 radios are not suitable for M17.  The audio path is not flat and the audio is band limited.  The [OpenRTX-project has a list of suitable radios](https://openrtx.org/#/M17/m17), but many of these radios need hardware modificiations in the audio path.  