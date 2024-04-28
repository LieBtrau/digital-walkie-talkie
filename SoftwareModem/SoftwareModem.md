# Overview
1. [Bell 202 1200bps](#Bell-202-1200bps) : slow
2. [Aicodix modem - Rattlegram - Ribbit](#Aicodix-modem---Rattlegram---Ribbit) : fast and versatile, but not suitable for audio streaming
3. [M17](#M17) : fast and suitable for audio streaming, but not suitable for PMR446 radios
4. [FDMDV](#FDMDV) : suitable for PMR446 radios, but can only be used with Codec2 audio data.  Audio output is not very intelligible.
5. [FreeDV 2400B](#FreeDV-2400B) : suitable for PMR446 radios, but can only be used with Codec2 audio data.  Audio output is not very intelligible.

# Bell 202 1200bps
`minimodem` - general-purpose software audio FSK modem is used to this purpose.  It's available from the Ubuntu package manager.

The Bell 202 1200bps modulation used in APRS to send packets over the 2m FM-band.  

## Test setup
We use the same hardware test setup as in [sending from FT65-E to G9Pro](FrequencyResponse.ipynb#Connection).
The sending station is the same as the receiving station.  The sending station sends data over the headphone output of the USB sound card.  The receiving station receives data from the microphone input of the same USB sound card.

The input audio settings of the sound card don't seem to matter much.  1% and 100% both decode the message, although it's better to use a setting that doesn't overdrive your analog input.
```bash
$ pactl set-source-volume alsa_input.usb-GeneralPlus_USB_Audio_Device-00.mono-fallback 1%
$ pactl set-source-volume alsa_input.usb-GeneralPlus_USB_Audio_Device-00.mono-fallback 100%
```
The analog output setting of your USB-sound card can be set to 100%
```bash
$ pactl set-sink-volume alsa_output.usb-GeneralPlus_USB_Audio_Device-00.analog-stereo 100%
```

## Test ✅
Send a text file from one station to the other.  On the receiving side, the output is printed out on the command line.

Sending station:
```bash
$ while true; do echo "The quick brown fox jumps over the lazy dog." | minimodem --tx 1200; done
```

Receiving station:
```bash
$ minimodem --rx 1200
```

## Result
Experiments have shown that 1200bps is about the maximum for this way of generating audio FSK using the Midland G9-Pro and the Yaesu FT65-E.  2400baud doesn't work.

----

# Aicodix modem - Rattlegram - Ribbit
This is the modem used in [Rattlegram](https://play.google.com/store/apps/details?id=com.aicodix.rattlegram&gl=US&pli=1)
To build it, have a look at the [installation instructions](#Installation-instructions) below, but checkout the `master`-branch instead of the `short`-branch.

Andreas Spiess made [a video about Rattlegram](https://www.youtube.com/watch?v=ubPP48ojJ3E).

* OFDM
* Selectable packet size
* Not suitable for audio streaming which requires low latency
* works with unmodified PMR446 radios (which have limited audio bandwidth)

This modem normally operates on files.  To make it decode continuously you can run:
```bash
while arecord -f S16_LE -c 1 -r 8000 - | ./decode - - ; do echo ; sleep 1 ; done
```

## Master version

### Offline test ✅
```bash
$ dd if=/dev/urandom of=uncoded.dat bs=1 count=5380
5380+0 records in
5380+0 records out
5380 bytes (5.4 kB, 5.3 KiB) copied, 0.027005 s, 199 kB/s
$ ./modem-master/encode encoded.wav 8000 16 1 uncoded.dat
real PAPR: 7.04375 .. 11.5406 dB
$ ./modem-master/decode decoded.dat encoded.wav 
symbol pos: 2298
coarse cfo: 2000 Hz 
oper mode: 6
call sign: ANONYMOUS
demod .................................................. done
coarse sfo: -0.0211067 ppm
finer cfo: 2000 Hz 
init Es/N0: 30.7753 dB
$ diff -s uncoded.dat decoded.dat 
Files uncoded.dat and decoded.dat are identical
```
Encoding 5380bytes results in 11s of audio.

```python
bytecount = 5380
audio_duration = 11
bitrate = bytecount * 8 / audio_duration
print('bitrate = {:n}bps'.format(bitrate))
```

## Loopback test on USB sound card ✅
### Audio level settings
Loop back audio out to MIC-IN of the USB Sound card using a TRRS-cable. Alsamixer setting :
* Speaker : 100
* MIC : 7
The tests have been performed with this setting.

### Alternative audio level settings
The `alsamixer`-settings that have been used in this test are unsuitable for the HTs.  The Yaesu FT65-E input audio signal should not be larger than 60mVpp, otherwise the audio will be distorted.
* Speaker : 7 (-37.5dB gain) → 53mVpp at input of Yaesu FT65-E.
  * The alsa speaker control changes with the main audio control in linux.
* Microphone : audio settings → input → 100%
  * The microphone level in alsa doesn't seem to do anything.  It's the linux sound settings input level which sets the microphone input level.

```bash
$ encode encoded.wav 8000 16 1 uncoded.dat 1450 NOCALL 13
$ arecord -c 1 -f S16_LE -r 8000 -d 30 recorded.wav
```
Meanwhile on another terminal:
```bash
aplay encoded.wav
```
And when `arecord` has finished:
```bash
$ ./modem-master/decode decoded.dat recorded.wav 
symbol pos: 13821
coarse cfo: 1450 Hz 
oper mode: 13
call sign:    NOCALL
demod .............................................................................................................................. done
coarse sfo: 0.0544232 ppm
finer cfo: 1450 Hz 
init Es/N0: 31.7376 dB
```
Remark that the Es/N0, aka. signal-to-noise ratio is around 30dB.  I had trouble with my test setup, yielding only 5 to 6dB.  In those cases the callsign could still be decoded, but data decoding failed.


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

----

# FDMDV - FreeDV 1600
This modem gets installed with codec2.  The tests have been done with the Ubuntu version of codec2.  

## Full digital loopback audio test
Other coding rates than 1400 for Codec2 are possible, but FreeDV uses 1400.  The following command line tools are used:
```bash
sox ../m17-tools/apollo11_1.wav -t raw - | c2enc 1400 - - | fdmdv_mod - - | fdmdv_demod - - | c2dec 1400 - - | play -q -b 16 -r 8000 -c1 -t s16 -
```

## Full digital loopback random data test
FDMDV is likely designed to work specifically with Codec2.  Is it possible to just send random data through it?
```bash
fdmdv_mod audio-codec.md - | fdmdv_demod - audio-codec_out.md 
diff audio-codec.md audio-codec_out.md 
Binary files audio-codec.md and audio-codec_out.md differ
```
The [FreeDV data modes](https://github.com/LieBtrau/codec2/blob/master/README_data.md) are designed for that purpose.  The most suitable mode for PMR446 radios is DATAC1 with an audio bandwidth of 1.7kHz.  And a payload rate of 980bps.  This will not be investiged further, because Rattlegram is already capable faster data rates.

## Inspect the FDMDV audio
Save the encoded audio to a WAV-file:
```bash
sox ../m17-tools/apollo11_1.wav -t wav - | c2enc 1400 - - | fdmdv_mod - - | sox -t raw -r 8000 -b 16 -c 1 -e signed-integer - fdmdv.wav
``` 
It can now be inspected in Audacity.  The spectrogram shows that the energy is in the 1-2.0kHz range so it should work on PMR446 radios.

## Real time loopback test
Test your radio setup first with REW: generate 1kHz audio and check with spectrogram that you can receive without distortion.

The computer gives an under-run error when trying to real-time encode and play the audio.  So the transmitter (Yaesu FT-65E) will play the audio we created in the previous step. 
```bash
play fdmdv.wav
```
The receiving station (Midland G9Pro) will record the audio and decode it in real-time.  The demodulator doesn't seem to check for energy level in the incoming audio.  It starts decoding noise, which generates junk audio when no signal is present.  Using the latest master build (on 2024-04-07) had the same issue.  M17 doesn't have this issue.
```bash
rec -c 1 -b 16 -t s16 -r 8000 - | fdmdv_demod - - | c2dec 1400 - - | aplay -c 1 -f S16_LE -r 8000 --device plughw:CARD=PCH,DEV=0
```
Sending FDMDV data over the PMR446 radio works.  The audio is decoded correctly, but is by times not very intelligible.  It would be better to have the option to use a higher coding rate (2400bps?) for codec2 and that the FDMDV-modem uses a wider audio bandwidth.  The PMR446 radios have more available bandwidth than the 1kHz used by FDMDV.  That should be possible with FreeDV.

# FreeDV 2400B
This mode is designed specifically for VHF analog FM.
You'll need to clone the [codec2 repository](https://github.com/drowe67/codec2) and build the tools yourself.

## Full digital loopback audio test
The mode can be tested on your PC using the following command line.  The modulated audio is passed through a 300Hz-3kHz bandpass filter to simulate the audio filters in the radios.  The audio is then demodulated and played back.:
```bash
$  ./freedv_tx 2400B ../../../m17-tools/apollo11_1.wav - | sox -t .s16 -r 48000 - -t .s16 - sinc 300-3000 | ./freedv_rx 2400B - - | play -t .s16 -r 8000 -
```
Remark that the modem sample rate is 48000Hz, but the audio sample rate is 8000Hz.  The audio is upsampled to 48000Hz before passing it to the modem.

## Loop back test on USB sound card
### Create the audio file
```bash
$  ./freedv_tx 2400B ../../../m17-tools/apollo11_1.wav - | sox -t .s16 -r 48000 - -t .s16 - sinc 300-3000 | sox -t raw -r 48000 -b 16 -c 1 -e signed-integer - freeDV_2400B.wav
```
### Record the audio and demodulate it
```bash
rec -c 1 -b 16 -t s16 -r 48000 - | ./freedv_rx 2400B - - | aplay -c 1 -f S16_LE -r 8000 --device plughw:CARD=PCH,DEV=0
```
### Play the audio
```bash
play freeDV_2400B.wav
```
Monitor the audio with REW Scope view to check if the audio is not distorted.  

The audio sounds good.  This modem also decodes noise when no signal is present.  It would be better if the modem would check for energy in the incoming audio.

## Real time loopback test using radios
### Hardware setup
Yaesu FT-65E sends audio to Midland G9Pro.  Input of Yaesu is connected to the output of the sound card.  Output of the Midland is connected to the input of the sound card.  Decoding works, but it's not very intelligible, maybe due to the buffer under-runs.  The audio has a lot of artefacts.
Recording the audio output from the Midland G9Pro first and then decoding it offline works much better.