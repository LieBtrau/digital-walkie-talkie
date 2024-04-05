# Codec options
As the walkie talkie will use digital voice transmission, we need a way to digitize speech.  Several open source speech codecs are available.  We will focus on low-bitrate codecs because we want long range.  Opus and Speex won't do.  There's one codec that excels : **codec2**.

## Codec2

* open source, royalty free replacement for AMBE.  At 2400bps, [AMBE+](https://www.rowetel.com/?p=5520) still performs better than Codec2.
* bitrates as low as 700bps possible (but not usable, see [Codec2 evaluation](https://hackaday.io/project/174231-digital-walkie-talkie/log/182246-codec2-installation-test).
* open source
* already implemented on embedded platforms : STM32, nRF52
* used in [FreeDV](https://freedv.org/)</a>, [QRadioLink](http://qradiolink.org/)

### Codec2 Configuration
Audio input format : 16bit signed integer, 8kHz sample rate, mono
Codec2 packet details (Referencen = [codec2 source code](https://github.com/blanu/codec2-arduino/blob/master/src/codec2/codec2.c)).



| Encoded Data rate [bps] | Bits/packet | Bytes/packet | Time interval [ms] | Packets/s |
|------------------------|-------------|--------------|--------------------|-----------|
| 3200                   | 64          | 8            | 20                 | 50        |
| 2400                   | 48          | 6            | 20                 | 50        |
| 1600                   | 64          | 8            | 40                 | 25        |
| 1400                   | 56          | 7            | 40                 | 25        |
| 1200                   | 48          | 6            | 40                 | 25        |

When using one of the lowest three data rates, there's a drawback that loosing a single packet will cost you 40ms of audio.

### Codec2 command line test
```bash
sox ../m17-tools/apollo11_1.wav -t raw - | c2enc 2400 - - | c2dec 2400 - - | play -q -b 16 -r 8000 -c1 -t s16 -
```

## Opus
* pen source, royalty free
* replacement for Speex
* down to 6kbps
* used in VoIP-applications (e.g. WhatsApp)

## MELPe
* NATO standard
* licensed & copyrighted

## Speech-to-text-to-speech
Using a speech codec, data transmission can be brought down to about 1200bps.  But how can we reduce data even further?  Let's take a 1min52s mono 8kHz speech sample as an example.

* [ve9qrp.wav](https://cdn.hackaday.io/files/1742317454299104/ve9qrp.wav) : : 1.799.212 bytes : **128000bps**
* ve9qrp.bin : codec2 1200bps encoded : 16.866 bytes : **1200bps**
* [ve9qrp.txt](https://cdn.hackaday.io/files/1742317454299104/ve9qrp.txt) : audio transcription of ve9qpr.wav : 1.178 bytes : **85bps**

Using codec2, we get a 106/1 compression ratio, using text 1527/1 compression ratio.   That's almost fifteen times better!

If we use speech recognition on the transmit side, then send the transcription over and use speech synthesis on the receiving side, this might work.


### Speech recognition
[https://pypi.org/project/SpeechRecognition/](https://pypi.org/project/SpeechRecognition/)

Even though speech recognition engines are very good these days, they're still not as good as human beings.  The transcript text could be shown to the speaker during transmission.  In case of errors, the speaker could repeat the incorrect word or spell out its characters.

A speech engine also requires a language preset.  That shouldn't be too much of a hurdle because most of us only commonly use a single language.

1. Is there good speech recognition software that runs offline?
2. Is speech recognition software not too power hungry?

### Speech synthesis
[Linux command line tools](https://hackaday.io/project/28445-usb-cdc-robosapien-v1/log/181633-text-to-speech)
